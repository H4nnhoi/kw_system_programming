#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

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

int main(){
    struct sockaddr_in server_addr, client_addr;
    int socket_fd, client_fd;
    int len, len_out;
    char buf[BUFFSIZE], cache_result[BUFFSIZE];
    pid_t pid;


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

    while (1)
    {
        struct in_addr inet_client_address;

        char response_header[BUFFSIZE] = {0, };
        char response_message[BUFFSIZE] = {0, };

        char tmp[BUFFSIZE] = {0, };
        char method[20] = {0, };
        char url[BUFFSIZE] = {0, };

        char *tok = NULL;

        len = sizeof(client_addr);
        client_fd = accept(socket_fd, (struct sockaddr*)&client_addr, &len);
        if (client_fd < 0)
        {
            printf("Server : accept failed\n");
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
            printf("[%s : %d] client was connected\n", inet_ntoa(inet_client_address), client_addr.sin_port);
            read(client_fd, buf, BUFFSIZE);
            strcpy(tmp, buf);

            inet_client_address.s_addr = client_addr.sin_addr.s_addr;

            puts("==============================================");
            printf("Request from [%s : %d]\n", inet_ntoa(inet_client_address), client_addr.sin_port);
            puts(buf);
            puts("==============================================");

            // HTTP 요청 파싱
            tok = strtok(tmp, " ");
            strcpy(method, tok);
            if (strcmp(method, "GET") == 0)
            {
                tok = strtok(NULL, " ");
                strcpy(url, tok);
            }

            int result = sub_process(url, &pid, log_fp, cachePath, sub_start_time, &hit_count, &miss_count);
            if(result == PROCESS_EXIT) break;
            else if(result == PROCESS_UNKNOWN){
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
                "Hello %s:%d<br><br>"
                "%s<br><br>"
                "kw2020202047"
                ,cache_result, inet_ntoa(inet_client_address), client_addr.sin_port, url);

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
            printf("[%s : %d] client was disconnected\n", inet_ntoa(inet_client_address), client_addr.sin_port);
           
        }
        close(client_fd);
    }

    close(socket_fd);
    return 0;
    
}