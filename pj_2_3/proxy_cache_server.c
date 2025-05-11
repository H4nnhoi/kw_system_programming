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

#include "sub_process.h"
#include "dirUtils.h"
#include "fileUtils.h"
#include "hit_and_miss.h"
#include "inputUtils.h"
#include "serverUtils.h"

#define BUFFSIZE 1024
#define PORTNO 40000
#define PROCESS_HIT 1
#define PROCESS_MISS 0
#define PROCESS_EXIT 7
#define PROCESS_UNKNOWN -1
#define LOGFILE_NAME_SIZE 13 

char home[BUFFSIZE];
char cachePath[BUFFSIZE];
char logPath[BUFFSIZE];
const char logfileName[LOGFILE_NAME_SIZE] = "logfile.txt";
time_t start_time;
time_t end_time;
time_t sub_start_time;
int hit_count;
int miss_count;
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
    // count init
    hit_count = 0;
    miss_count = 0;
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
//////////////////////////////////////////////////////////////////////////
// File Name : proxy_cache_server.c                                     //
// Date      : 2025/05/08                                               //
// OS        : Ubuntu / macOS                                           //
// Author    : 이정한                                                     //
// -------------------------------------------------------------------- //
// Title     : System Programming Assignment #2-2 (Proxy Server)        //
// Description :                                                        //
//   This proxy server handles HTTP requests from a web browser.        //
//   Each request is processed by a child process created with fork().  //
//   - Extracts the URL from the HTTP request                           //
//   - Checks for a cached file based on the hashed URL                 //
//   - If HIT: responds with cached content & write log of hit contents //
//   - If MISS: connects to target web server, stores and responds      //
//                                        (+ write log of miss contents)//
//   - Uses SHA1 hashing for cache filenames                            //
//   - Tracks HIT/MISS statistics and logs each request                 //
//   - Handles zombie processes using SIGCHLD signal handler            //
//   - Displays internal IP and port for each client connection         //
//////////////////////////////////////////////////////////////////////////

int main(){
    struct sockaddr_in server_addr, client_addr;
    int socket_fd, client_fd;
    int len, len_out;
    char buf[BUFFSIZE], cache_result[BUFFSIZE];
    pid_t pid;
    vars_setting();


    if ((socket_fd = socket(PF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("Server : Can't open stream socket\n");
        return 0;
    }

    bzero((char *)&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(PORTNO);

    if (bind(socket_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        printf("Server : Can't bind local address\n");
        return 0;
    }

    listen(socket_fd, 5);
    signal(SIGCHLD, (void *)handler);  // 자식 프로세스 종료 처리


    while (1)
    {
        struct in_addr inet_client_address;

        char response_header[BUFFSIZE] = {0, };
        char response_message[BUFFSIZE] = {0, };

        char tmp[BUFFSIZE] = {0, };
        char method[20] = {0, };
        char* url;

        char *tok = NULL;
        char *internel_ip = get_internal_ip();

        len = sizeof(client_addr);
        client_fd = accept(socket_fd, (struct sockaddr*)&client_addr, &len);
        // error 1. not accepted
        if (client_fd < 0)
        {
            printf("Server : accept failed\n");
            return 0;
        }
        pid = fork();

        // error 2. can't execute "fork"
        if (pid == -1) {
            close(client_fd);
            close(socket_fd);
            continue;
        }

        if (pid == 0) {  // 자식 프로세스
            time_t sub_start_time;
            time(&sub_start_time);
            printf("[%s : %d] client was connected\n", internel_ip, client_addr.sin_port);
            ssize_t n = read(client_fd, buf, BUFFSIZE);
            // error 3. read failed
            if (n <= 0) {
                perror("read failed or client closed connection");
                close(client_fd);
                exit(1);
            }
            strcpy(tmp, buf);

            inet_client_address.s_addr = client_addr.sin_addr.s_addr;

            puts("==============================================");
            printf("Request from [%s : %d]\n", internel_ip, client_addr.sin_port);
            puts(buf);
            puts("==============================================");

            url = get_parsing_url(tmp);
            printf("url = %s\n", url);

            int result = sub_process(internel_ip, client_addr.sin_port, url, &pid, log_fp, cachePath, sub_start_time, &hit_count, &miss_count);
            if(result == PROCESS_EXIT) break;
            else if(result == PROCESS_UNKNOWN){ // error 5. unknown error in subprocess
                perror("Error of subprocess");
                break;
            }else if(result == PROCESS_HIT){
                sprintf(cache_result, "HIT");
            }else if(result == PROCESS_MISS){
                sprintf(cache_result, "MISS");
            } 

            // 응답 본문
            sprintf(response_message,
                "<h1>%s</h1><br>"
                "%s:%d<br>"
                "%s<br>"
                "kw2020202047"
                ,cache_result, internel_ip, client_addr.sin_port, url);

            // HTTP 헤더 작성
            sprintf(response_header,
                "HTTP/1.0 200 OK\r\n"
                "Server:2018 simple web server\r\n"
                "Content-length:%lu\r\n"
                "Content-type:text/html\r\n\r\n",
                strlen(response_message));

            // 전송
            write(client_fd, response_header, strlen(response_header));
            write(client_fd, response_message, strlen(response_message));
            printf("[%s : %d] client was disconnected\n", internel_ip, client_addr.sin_port);
            exit(0);
        }
        close(client_fd);
    }

    close(socket_fd);
    return 0;
    
}