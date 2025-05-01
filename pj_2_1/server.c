#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/wait.h>
#include <time.h>
#include <arpa/inet.h> 
#include "proxy_cache.h"
#include "dirUtils.h"
#include "fileUtils.h"
#include "hit_and_miss.h"
#include "inputUtils.h"

#define BUFFSIZE 1024
#define PORTNO 40000

#define MAX_INPUT 256
#define LOGFILE_NAME_SIZE 13 
#define CACHE_DIR_SIZE 4
#define FILE_SIZE 40
#define PROCESS_HIT 1
#define PROCESS_MISS 0
#define PROCESS_EXIT 7
#define PROCESS_UNKNOWN -1

char home[MAX_INPUT];
char cachePath[MAX_INPUT];
char logPath[MAX_INPUT];
const char logfileName[LOGFILE_NAME_SIZE] = "logfile.txt";
time_t start_time;
time_t end_time;
time_t sub_start_time;
int hit_count;
int miss_count;
int sub_process_count;
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
    snprintf(cachePath, MAX_INPUT, "%s/cache", home);
    snprintf(logPath, MAX_INPUT, "%s/logfile", home);
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
// File Name : server.c                                              //
// Date      : 2025/04/30                                            //
// OS        : Ubuntu / macOS                                        //
// Author    : (Your Name)                                           //
// ----------------------------------------------------------------- //
// Title     : System Programming Assignment #1-3 (Proxy Server)     //
// Description :                                                     //
//   This server program implements a simple HTTP proxy server       //
//   that receives URLs from clients over a TCP connection.          //
//   Each request is handled by a forked child process.              //
//   - URLs are hashed using SHA1 and checked in a cache directory   //
//   - HIT or MISS is logged and responded to the client             //
//   - Logs include timestamps and PIDs for tracking                 //
//   - The server gracefully handles termination via signals         //
///////////////////////////////////////////////////////////////////////
int main() {
    struct sockaddr_in server_addr, client_addr;
    int socket_fd, client_fd;
    int len, len_out;
    char buf[BUFFSIZE];
    char hadddr[] = "127.0.0.1";
    pid_t pid;
    vars_setting();

    if ((socket_fd = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
        printf("Server : Can't open stream socket\n");
        return 0;
    }

    bzero((char*)&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(hadddr);
    server_addr.sin_port = htons(PORTNO);

    if (bind(socket_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        printf("Server : Can't bind local address\n");
        close(socket_fd);
        return 0;
    }

    listen(socket_fd, 5);
    signal(SIGCHLD, (void *)handler);  // 자식 프로세스 종료 처리

    while (1) {
        bzero((char*)&client_addr, sizeof(client_addr));
        len = sizeof(client_addr);
        client_fd = accept(socket_fd, (struct sockaddr*)&client_addr, &len);

        if (client_fd < 0) {
            printf("Server : accept failed %d\n", getpid());
            close(socket_fd);
            return 0;
        }

        printf("[%s : %d] client was connected\n", inet_ntoa(client_addr.sin_addr), client_addr.sin_port);
        pid = fork();

        if (pid == -1) {
            close(client_fd);
            close(socket_fd);
            continue;
        }

        if (pid == 0) {  // 자식 프로세스
            time_t sub_start_time;
            time(&sub_start_time);
            while(1){
                int msg_len;
                recv(client_fd, &msg_len, sizeof(int), 0);
                char input_url[msg_len];
                recv(client_fd, &input_url, msg_len, 0);
                int result = sub_process(input_url, &pid, log_fp, cachePath, sub_start_time, &hit_count, &miss_count);
                if(result == PROCESS_EXIT) break;
                else if(result == PROCESS_UNKNOWN){
                    perror("Error of subprocess");
                    break;
                }else if(result == PROCESS_HIT){
                    send(client_fd, "HIT\n", strlen("HIT\n"), 0);
                    continue;
                }else if(result == PROCESS_MISS){
                    send(client_fd, "MISS\n", strlen("MISS\n"), 0);
                    continue;
                } 
            }

            printf("[%d : %d] client was disconnected\n", client_addr.sin_addr.s_addr, client_addr.sin_port);
            close(client_fd);
            exit(0);
        }
        close(client_fd);  // 부모 프로세스는 클라이언트 소켓 닫음
    }
    close(socket_fd);
    return 0;
}
