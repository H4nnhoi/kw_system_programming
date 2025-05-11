#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <sys/types.h>
#include <unistd.h>
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

///////////////////////////////////////////////////////////////////////
// sub_process                                                       //
///////////////////////////////////////////////////////////////////////
// Input:                                                            //
//   - char* input_url : requested URL from the client               //
//   - pid_t* PID : process ID of the child process                  //
//   - FILE* log_fp : log file pointer                               //
//   - const char* cachePath : base path of the cache directory      //
//   - time_t sub_start_time : time when the process started         //
//   - int* hit_count : pointer to HIT counter                       //
//   - int* miss_count : pointer to MISS counter                     //
// Output:                                                           //
//   - int : PROCESS_HIT / PROCESS_MISS / PROCESS_EXIT / ERROR CODE  //
// Purpose:                                                          //
//   - Hash the URL and check if it's a cache HIT or MISS            //
//   - Create cache file if MISS, log all activity                   //
///////////////////////////////////////////////////////////////////////
int sub_process(char* hostname, int port, char* input_url, pid_t* PID, FILE *log_fp, const char *cachePath, time_t sub_start_time, int *hit_count, int *miss_count){
    // setting sub_process
    char hashed_url[41];
    char subdir[CACHE_DIR_SIZE];
    char fileName[FILE_SIZE];
    char* log_contents = NULL;
    char* subCachePath = NULL;
    

    if (input_url == NULL) {
        perror("input is null");
        return PROCESS_UNKNOWN;
    }
            
    // get hashed URL using sha1_hash function
    sha1_hash(input_url, hashed_url);

    // divide hashed_url
    strncpy(subdir, hashed_url, 3);
    subdir[3] = '\0';

    // create file by divide hashed_url
    // * edit file path range [3-40]
    strncpy(fileName, hashed_url + 3, sizeof(fileName) - 1);
    fileName[sizeof(fileName) - 1] = '\0'; 

    subCachePath = make_dir_path(cachePath, subdir);

    int hit_and_miss_result = is_file_hit(subCachePath, fileName);

    // HIT & MISS case
    if(hit_and_miss_result == PROCESS_MISS){      // MISS
        int server_fd = connect_to_webserver(hostname, port);
        if (server_fd < 0) {
            fprintf(stderr, "Cannot connect to web server: %s\n", hostname);
            free(subCachePath);
            return PROCESS_UNKNOWN;
        }
        char request_buf[BUFFSIZE];
        snprintf(request_buf, sizeof(request_buf),
            "GET %s HTTP/1.0\r\n"
            "Host: %s\r\n"
            "Connection: close\r\n\r\n",
            input_url, hostname);

        printf("request : %s\n", request_buf);

        if (send_http_request(server_fd, request_buf) < 0) {
            close(server_fd);
            free(subCachePath);
            return PROCESS_UNKNOWN;
        }

        char buffer[BUFFSIZE];
        receive_http_response(server_fd, buffer, sizeof(buffer));

        printf("response = %s\n", buffer);
        log_contents = get_miss_log(input_url);
        ensureDirExist(subCachePath, 0777);
        createFile(subCachePath, fileName);
        (*miss_count)++;
    }else if(hit_and_miss_result == PROCESS_HIT){      // HIT
        char* hashed_path = make_dir_path(subdir, fileName);
        log_contents = get_hit_log(hashed_path, input_url);
        free(hashed_path);
        (*hit_count)++;
    }else{
        perror("not range of return\n");
        free(input_url);
        free(subCachePath);
        return PROCESS_UNKNOWN;
    }
    write_log_contents(log_fp, log_contents);

    //free memory for dynamic allocation
    free(subCachePath);
    free(log_contents);
    
    return hit_and_miss_result;
}
