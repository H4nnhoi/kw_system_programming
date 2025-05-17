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

#define MAX_INPUT 256
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

//////////////////////////////////////////////////////////////////////////
// sub_process                                                          //
//////////////////////////////////////////////////////////////////////////
// Input:                                                               //
//   - char* input_url       : The full URL requested by the client     //
//   - pid_t* PID            : Pointer to the current child process ID  //
//   - FILE* log_fp          : File pointer for the server log          //
//   - const char* cachePath : Root directory path for cached files     //
//   - time_t sub_start_time : Timestamp when the child process started //
//   - int* hit_count        : Pointer to HIT counter                   //
//   - int* miss_count       : Pointer to MISS counter                  //
//   - char* buf             : Buffer to store the HTTP response        //
//   - size_t buf_size       : Maximum size of the response buffer      //
// Return:                                                              //
//   - PROCESS_HIT           : Returned if cache HIT                    //
//   - PROCESS_MISS          : Returned if cache MISS                   //
//   - PROCESS_EXIT          : Returned on timeout or forced exit       //
//   - PROCESS_UNKNOWN       : Returned on other internal errors        //
// Description:                                                         //
//   - Hashes the URL using SHA1 to generate a unique cache filename    //
//   - Checks whether the file exists in the cache directory            //
//   - On HIT: reads the cached file and stores its content in buffer   //
//   - On MISS:                                                         //
//       • Resolves IP address of the target host                       //
//       • Connects to the web server via socket                        //
//       • Sends an HTTP GET request and waits for response (10s)       //
//       • Caches the received response to a file                       //
//   - Logs each request with timestamp, URL, and HIT/MISS status       //
//   - Frees any dynamically allocated resources                        //
//////////////////////////////////////////////////////////////////////////
int sub_process(char* input_url, pid_t* PID, FILE *log_fp, const char *cachePath, time_t sub_start_time,
                int *hit_count, int *miss_count, char* buf, size_t buf_size) {
    if (input_url == NULL) {
        perror("input is null");
        return PROCESS_UNKNOWN;
    }

    char hashed_url[41], subdir[CACHE_DIR_SIZE], fileName[FILE_SIZE];
    char* log_contents = NULL;
    char* subCachePath = NULL;
    char* trimmed_url = input_url + 7;

    sha1_hash(input_url, hashed_url);
    strncpy(subdir, hashed_url, 3); subdir[3] = '\0';
    strncpy(fileName, hashed_url + 3, sizeof(fileName) - 1); fileName[sizeof(fileName) - 1] = '\0';
    subCachePath = make_dir_path(cachePath, subdir);
    snprintf(cache_full_path, sizeof(cache_full_path), "%s/%s", subCachePath, fileName);
    char* full_path = make_dir_path(subCachePath, fileName);

    int result = is_file_hit(subCachePath, fileName);
    FILE* cache_fp = NULL;

    signal(SIGALRM, timeout_handler);

    if (result == PROCESS_MISS) {
        // ready to write cache file
        ensureDirExist(subCachePath, 0777);
        createFile(subCachePath, fileName);
        // get host ip address
        char* hostIpAddr = getIPAddr(trimmed_url);
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
            "GET %s HTTP/1.0\r\n"
            "Host: %s\r\n"
            "Connection: close\r\n\r\n",
            input_url, hostIpAddr);
        //START ALARM COUNT
        alarm(10);
        if (send_http_request(server_fd, request_buf) < 0) {
            close(server_fd);
            free(subCachePath);
            return PROCESS_UNKNOWN;
        }
        // write response to buf & cache file
        init_log(&cache_fp, cache_full_path);
        receive_http_response(server_fd, buf, buf_size);
        alarm(0);
        write_log_contents(cache_fp, buf);
        close_log(cache_fp);
        // write miss log
        log_contents = get_miss_log(input_url);
        (*miss_count)++;

    } else if (result == PROCESS_HIT) {
        // write response to buf
        init_log(&cache_fp, cache_full_path);
        if (!cache_fp) {
            perror("fopen failed");
            free(subCachePath);
            return PROCESS_UNKNOWN;
        }
        read_file_to_buffer(cache_fp, buf, buf_size);
        close_log(cache_fp);
        
        // write hit log
        log_contents = get_hit_log(full_path, input_url);
        (*hit_count)++;

    } else {
        perror("not range of return\n");
        free(input_url);
        free(subCachePath);
        return PROCESS_UNKNOWN;
    }

    write_log_contents(log_fp, log_contents);
    free(subCachePath);
    free(log_contents);
    free(full_path);

    return result;
}

