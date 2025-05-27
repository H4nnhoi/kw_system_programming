#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <time.h>
#include <stdlib.h>
#include <ifaddrs.h>
#include <net/if.h>
#include <signal.h>

#include "sub_process.h"
#include "dirUtils.h"
#include "fileUtils.h"
#include "hit_and_miss.h"
#include "inputUtils.h"
#include "serverUtils.h"

#define BUFFSIZE 1024
#define RESPONSE_SIZE 2048
#define PORTNO 39999
#define MAIN_REQUEST 1
#define SUB_REQUEST 0
#define PROCESS_UNKNOWN -1
#define LOGFILE_NAME_SIZE 13 

char home[BUFFSIZE];
char cachePath[BUFFSIZE];
char logPath[BUFFSIZE];
const char logfileName[LOGFILE_NAME_SIZE] = "logfile.txt";
const char *filter_keywords[] = {
    "firewall", "socket",
    "goog", "ocsp", "r11", "r10", "firefox"
};
const int filter_keyword_count = sizeof(filter_keywords) / sizeof(filter_keywords[0]);
time_t start_time;
time_t end_time;
int process_count = 0;
int pipefd[2];
FILE *log_fp;

///////////////////////////////////////////////////////////////////////
// vars_setting                                                      //
///////////////////////////////////////////////////////////////////////
// Input:    None                                                    //
// Output:   None                                                    //
// Purpose:                                                          //
//   - Initializes global variables and time tracking                //
//   - Prepares cache and log directories                            //
//   - Checks if logfile exists, creates it if not                   //
//   - Opens the logfile in append mode using init_log()             //
///////////////////////////////////////////////////////////////////////
void vars_setting(){
    // time log init
    time(&start_time);
    
    // value setting(empty input, home path, cache path)
    getHomeDir(home);
    snprintf(cachePath, BUFFSIZE, "%s/cache", home);
    snprintf(logPath, BUFFSIZE, "%s/logfile", home);
    // if cache&log directory didnt exsit, make directory permit 777
    ensureDirExist(cachePath, 0777);
    ensureDirExist(logPath, 0777);
    
    // check logfile.txt exist and make file
    if(is_file_hit(logPath, logfileName) == 0){
        createFile(logPath, logfileName);
    }
    //open file
    char* log_full_path = make_dir_path(logPath, logfileName);
    // printf("log_full_path %s\n", log_full_path);
    init_log(&log_fp, log_full_path);
    free(log_full_path);
}
///////////////////////////////////////////////////////////////////////
// get_internal_ip                                                   //
///////////////////////////////////////////////////////////////////////
// Input:    None                                                    //
// Output:   char* – internal IPv4 address in dotted-decimal format  //
// Purpose:                                                          //
//   - Retrieves the first non-loopback IPv4 address of the system   //
//   - Iterates through network interfaces using getifaddrs()        //
//   - Returns "Unknown" if no valid address is found                //
//   - Used for identifying the proxy server’s internal IP           //
///////////////////////////////////////////////////////////////////////
char* get_internal_ip() {
    static char ip[INET_ADDRSTRLEN] = "Unknown";
    struct ifaddrs *ifaddr, *ifa;

    if (getifaddrs(&ifaddr) == -1) {
        perror("getifaddrs failed");
        return ip;
    }

    for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) {
        if (ifa->ifa_addr && ifa->ifa_addr->sa_family == AF_INET &&
            !(ifa->ifa_flags & IFF_LOOPBACK)) {
            struct sockaddr_in *sa = (struct sockaddr_in *)ifa->ifa_addr;
            inet_ntop(AF_INET, &(sa->sin_addr), ip, INET_ADDRSTRLEN);
            break;
        }
    }
    freeifaddrs(ifaddr);
    return ip;
}
///////////////////////////////////////////////////////////////////////
// handler                                                           //
///////////////////////////////////////////////////////////////////////
// Input:    None                                                    //
// Output:   None                                                    //
// Purpose:                                                          //
//   - Handles SIGCHLD signal by reaping zombie child processes      //
//   - Uses waitpid() with WNOHANG to clean up non-blockingly        //
///////////////////////////////////////////////////////////////////////
static void handler() {
    pid_t pid;
    int status;
    while ((pid = waitpid(-1, &status, WNOHANG)) > 0);

}
///////////////////////////////////////////////////////////////////////
// interrupt_handler                                                 //
///////////////////////////////////////////////////////////////////////
// Input:    None                                                    //
// Output:   None                                                    //
// Purpose:                                                          //
//   - Handles SIGINT (Ctrl+C) signal to terminate server            //
//   - Calculates server uptime using start and end timestamps       //
//   - Generates a termination log summarizing session info          //
//   - Writes the termination log to the server log file             //
//   - Closes the log file before exiting the program                //
///////////////////////////////////////////////////////////////////////
static void interrupt_handler(){
    char* terminate_log = NULL;
    time(&end_time);
    terminate_log = get_server_terminated_log(difftime(end_time, start_time), process_count);
    write_log_contents(log_fp, terminate_log);
    close_log(terminate_log);
}

///////////////////////////////////////////////////////////////////////
// is_filtered_url                                                   //
///////////////////////////////////////////////////////////////////////
// Input:                                                            //
//   - const char* url : The URL string from the HTTP request        //
// Return:                                                           //
//   - int            : Returns 1 if the URL should be filtered      //
//                      (i.e., ignored and not processed)            //
//                      Returns 0 if the URL is valid to process     //
// Purpose:                                                          //
//   - Determines whether the incoming URL contains unwanted         //
//     resource patterns (e.g., favicon, analytics, scripts)         //
//   - Helps reduce unnecessary resource handling in the proxy       //
//   - Common targets include background assets and tracking links   //
// Implementation:                                                   //
//   - Uses strstr() to search for known keyword patterns            //
//   - Keywords are stored in a static array for easy maintenance    //
///////////////////////////////////////////////////////////////////////
int is_filtered_url(const char *url) {
    for (int i = 0; i < filter_keyword_count; i++) {
        if (strstr(url, filter_keywords[i]) != NULL) {
            return 1; // 필터링 대상
        }
    }
    return 0;
}

//////////////////////////////////////////////////////////////////////////
// File Name : proxy_cache_server.c                                     //
// Date      : 2025/05/20                                               //
// OS        : Ubuntu                                                   //
// Author    : 이정한                                                     //
//////////////////////////////////////////////////////////////////////////
// Title     : System Programming Assignment #2-4 (Proxy Server)        //
// Description :                                                        //
//   This proxy server handles HTTP GET requests from web browsers.     //
//   Each client connection is handled by a forked child process.       //
//                                                                      //
//   [Core Functionalities]                                             //
//   - Creates a TCP socket server and waits for incoming connections   //
//   - Accepts client HTTP requests and extracts the target URL         //
//   - Filters out unnecessary or background requests (e.g., favicon)   //
//   - Forks a child process for each valid request                     //
//     • Each child calls sub_process() to handle cache logic           //
//       - If cache HIT: Reads from file and sends response             //
//       - If cache MISS: Connects to remote server, stores response    //
//   - Uses SHA1 hashing to generate unique cache filenames             //
//   - Manages timeout handling with SIGALRM for slow responses         //
//   - Prevents zombie processes using SIGCHLD signal handler           //
//   - Supports graceful shutdown with SIGINT (Ctrl+C)                  //
//   - Displays internal IP and client port for each request            //
//////////////////////////////////////////////////////////////////////////
int main(){
    
    struct sockaddr_in server_addr, client_addr;
    int socket_fd, client_fd;
    int len, len_out;
    char buf[BUFFSIZE];
    pid_t pid;
    vars_setting();


    if ((socket_fd = socket(PF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("Server : Can't open stream socket\n");
        return 0;
    }

    bzero((char *)&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(PORTNO);

    if (bind(socket_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        perror("Server : Can't bind local address\n");
        return 0;
    }

    listen(socket_fd, 10);
    signal(SIGINT, (void *) interrupt_handler);     // when interrupted process by cntrl + c
	signal(SIGCHLD, (void *) handler);
    while (1)
    {
        struct in_addr inet_client_address;

        char response_header[BUFFSIZE] = {0, };
        char response_message[RESPONSE_SIZE];

        char tmp[BUFFSIZE] = {0, };
        char method[20] = {0, };
        char* url;

        char *tok = NULL;
        char *internel_ip = get_internal_ip();

        len = sizeof(client_addr);
        client_fd = accept(socket_fd, (struct sockaddr*)&client_addr, &len);
        if (client_fd < 0)
        {
            perror("Server : accept failed\n");
            return 0;
        }

        ssize_t n = read(client_fd, buf, BUFFSIZE);
        // error. read failed
        if (n <= 0) {
            perror("read failed or client closed connection");
            close(client_fd);
            continue;
        }
        strcpy(tmp, buf);
        inet_client_address.s_addr = client_addr.sin_addr.s_addr;
        url = get_parsing_url(tmp);
        size_t len = strlen(url);
        
	    puts(buf);

        //block not request url
        if (is_filtered_url(url)) {
            close(client_fd);
            continue;
        }
        pid = fork();

        // error. can't execute "fork"
        if (pid == -1) {
            close(client_fd);
            close(socket_fd);
            continue;
        }

        if (pid == 0) {  // 자식 프로세스
	    //close(pipefd[0]);
	    int report = 0;
            time_t sub_start_time;
            time(&sub_start_time);

            int result = sub_process(url, log_fp, cachePath, client_fd);
	    if(result == MAIN_REQUEST){
		    report++;
	    }
	    //write(pipefd[1], &report, sizeof(report));
	    //close(pipefd[1]);
            exit(0);
        }
	close(client_fd);
	free(url);
	//close(pipefd[1]);
    }

    close(socket_fd);
    return 0;
    
}
