#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <sys/types.h>
#include <unistd.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <signal.h>
#include "sha1Utils.h"
#include "dirUtils.h"
#include "fileUtils.h"
#include "hit_and_miss.h"
#include "inputUtils.h"
#include "serverUtils.h"

#define MAX_INPUT 4096
#define BUFFSIZE 2048
#define LOGFILE_NAME_SIZE 13 
#define CACHE_DIR_SIZE 4
#define FILE_SIZE 40
#define PROCESS_HIT 1
#define PROCESS_MISS 0
#define PROCESS_EXIT 7
#define PROCESS_UNKNOWN -1
#define DEFAULT_PORT 80

char cache_full_path[1024];

///////////////////////////////////////////////////////////////////////
// getIPAddr                                                         //
///////////////////////////////////////////////////////////////////////
// Input:                                                            //
//   - char* addr : Hostname (e.g., "www.example.com")               //
// Return:                                                           //
//   - char*      : String representation of the IPv4 address        //
//                  (e.g., "93.184.216.34")                          //
//                  Returns NULL on failure                          //
// Description:                                                      //
//   - Resolves a hostname to its corresponding IPv4 address using   //
//     DNS via gethostbyname().                                      //
//   - Converts the resolved address to a human-readable string      //
//     using inet_ntoa().                                            //
//   - On failure, prints DNS resolution error via herror().         //
///////////////////////////////////////////////////////////////////////
char *getIPAddr(char *addr)
{
    struct hostent* hent;
    if ((hent = gethostbyname(addr)) != NULL) {
        return inet_ntoa(*((struct in_addr*)hent->h_addr_list[0]));
    } else {
        herror("gethostbyname failed");
        return NULL;
    }
} 
///////////////////////////////////////////////////////////////////////
// timeout_handler                                                   //
///////////////////////////////////////////////////////////////////////
// Input:                                                            //
//   - int signum : Signal number (e.g., SIGALRM)                    //
// Description:                                                      //
//   - This function is triggered when a timeout signal (SIGALRM)    //
//     is received.                                                  //
//   - It prints a timeout message to stderr and exits the child     //
//     process with PROCESS_EXIT code.                               //
///////////////////////////////////////////////////////////////////////
void timeout_handler(int signum) {
    fprintf(stderr, "==========NO RESPONSE==========\n");
    exit(PROCESS_EXIT);  // 자식 프로세스 종료
}


///////////////////////////////////////////////////////////////////////////
// sub_process                                                           //
///////////////////////////////////////////////////////////////////////////
// Input:                                                                //
//   - char* input_url       : The full URL requested by the client      //
//   - FILE* log_fp          : File pointer to the main server log       //
//   - const char* cachePath : Root directory path for cached files      //
//   - int client_fd         : Socket file descriptor for client(browser)//
// Output:                                                               //
//   - int                   : PROCESS_HIT if served from cache          //
//                             PROCESS_MISS if fetched from web server   //
//                             PROCESS_UNKNOWN on internal failure       //
//                             PROCESS_EXIT on timeout                   //
// Purpose:                                                              //
//   - Handles an individual client request in a forked subprocess       //
//   - Parses the URL into host/path and hashes it to a cache key        //
//   - Determines whether the requested resource is already cached       //
//   - If HIT:                                                           //
//       • Reads cached file into memory and sends to client             //
//   - If MISS:                                                          //
//       • Resolves host IP, connects to web server                      //
//       • Sends HTTP GET request                                        //
//       • Receives full HTTP response                                   //
//       • Sends response to client and caches it to a file              //
//   - Logs the request with timestamp, status (HIT/MISS), and URL       //
//   - Handles slow responses using alarm() for timeout protection       //
//   - Frees all dynamically allocated memory before returning           //
///////////////////////////////////////////////////////////////////////////
int sub_process(char* input_url, FILE *log_fp, const char *cachePath, int client_fd) {
    if (input_url == NULL) {
        perror("input is null");
        return PROCESS_UNKNOWN;
    }

    char hashed_url[41], subdir[CACHE_DIR_SIZE], fileName[FILE_SIZE];
    char* log_contents = NULL;
    char* subCachePath = NULL;
    char trimmed_url[BUFFSIZE];
    char host[512], path[512];

    get_parsing_host_and_path(input_url, host, path);
    snprintf(trimmed_url, BUFFSIZE, "%s%s", host, path);

    sha1_hash(trimmed_url, hashed_url);
    strncpy(subdir, hashed_url, 3); subdir[3] = '\0';
    strncpy(fileName, hashed_url + 3, sizeof(fileName) - 1); fileName[sizeof(fileName) - 1] = '\0';
    subCachePath = make_dir_path(cachePath, subdir);
    snprintf(cache_full_path, sizeof(cache_full_path), "%s/%s", subCachePath, fileName);
    char* full_path = make_dir_path(subCachePath, fileName);

    int result = is_file_hit(subCachePath, fileName);
    FILE* cache_fp = NULL;

    signal(SIGALRM, timeout_handler);

    if (result == PROCESS_MISS) {
        
        // get host ip address
        char* hostIpAddr = getIPAddr(host);
        if (!hostIpAddr) {
            fprintf(stderr, "gethostbyname failed for %s\n", trimmed_url);
            free(subCachePath);
            return PROCESS_UNKNOWN;
        }
        // try to connect host
        int server_fd = connect_to_webserver(hostIpAddr, DEFAULT_PORT);
        if (server_fd < 0) {
            fprintf(stderr, "Cannot connect to web server: %s\n", hostIpAddr);
            free(subCachePath);
            return PROCESS_UNKNOWN;
        }
        // set request & send request message to host
        char request_buf[BUFFSIZE];
        snprintf(request_buf, sizeof(request_buf),
            "GET %s HTTP/1.1\r\n"
            "Host: %s\r\n"
            "User-Agent: Mozilla/5.0 (X11; Ubuntu; Linux x86_64; rv:136.0) Gecko/20100101 Firefox/136.0\r\n"
            "Accept: text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8\r\n"
            "Accept-Language: en-US,en;q=0.5\r\n"
            "Accept-Encoding: identity\r\n"
            "Upgrade-Insecure-Requests: 1\r\n"
            "Connection: close\r\n\r\n",
            path, host);
        //START ALARM COUNT
        alarm(20);
	    //sleep(5);
        if (send_http_request(server_fd, request_buf) < 0) {
            close(server_fd);
            free(subCachePath);
            return PROCESS_UNKNOWN;
        }
	
        // write response to buf & cache file
        ensureDirExist(subCachePath,0777);
        createFile(subCachePath, fileName);
        init_log(&cache_fp, cache_full_path);

        size_t response_size;
        char* response = receive_http_response(server_fd, &response_size);
        send(client_fd, response, strlen(response), 0);
        alarm(0);
        write_log_contents(cache_fp, response);
        close_log(cache_fp);
        // write miss log
        log_contents = get_miss_log(trimmed_url);
        free(response);

    } else if (result == PROCESS_HIT) {
        // write response to buf
        init_log(&cache_fp, cache_full_path);
        if (!cache_fp) {
            perror("fopen failed");
            free(subCachePath);
            return PROCESS_UNKNOWN;
        }
	    size_t file_size;
        char *buf = read_file_to_dynamic_buffer(cache_fp, &file_size);

        if (buf) {
            send(client_fd, buf, file_size, 0);
            free(buf);
        } else {
            fprintf(stderr, "Failed to read cache file into memory.\n");
        }
        close_log(cache_fp);
        // write hit log
        log_contents = get_hit_log(full_path, trimmed_url);

    } else {
        perror("not range of return\n");
        free(input_url);
        free(subCachePath);
        return PROCESS_UNKNOWN;
    }
    printf("log : %s\n", log_contents);
    write_log_contents(log_fp, log_contents);
    free(subCachePath);
    free(log_contents);
    free(full_path);

    return result;
}

