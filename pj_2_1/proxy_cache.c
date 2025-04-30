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

#define MAX_INPUT 256
#define LOGFILE_NAME_SIZE 13 
#define CACHE_DIR_SIZE 4
#define FILE_SIZE 40
#define CMD_REPEAT 1
#define CMD_EXIT 0
#define CMD_UNKNOWN -1


int sub_process(char* input_url, pid_t* PID, FILE *log_fp, const char *cachePath, time_t sub_start_time, int *hit_count, int *miss_count){
    // setting sub_process
    char hashed_url[41];
    char subdir[CACHE_DIR_SIZE];
    char fileName[FILE_SIZE];
    char* log_contents = NULL;
    char* subCachePath = NULL;
    

    if (input_url == NULL) {
        perror("input is null");
        return CMD_UNKNOWN;
    }
            
    // end cmd
    if (strcmp(input_url, "bye") == 0) {
        time_t sub_end_time;
        time(&sub_end_time);
        char* terminate_log = get_terminated_log(difftime(sub_end_time, sub_start_time), *hit_count, *miss_count);
        write_log_contents(log_fp, terminate_log);
        free(terminate_log);
         return CMD_EXIT;        
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
    if(hit_and_miss_result == 0){      // MISS
        log_contents = get_miss_log(input_url);
        ensureDirExist(subCachePath, 0777);
        createFile(subCachePath, fileName);
        (*miss_count)++;
    }else if(hit_and_miss_result == 1){      // HIT
        char* hashed_path = make_dir_path(subdir, fileName);
        log_contents = get_hit_log(hashed_path, input_url);
        free(hashed_path);
        (*hit_count)++;
    }else{
        perror("not range of return\n");
        free(input_url);
        free(subCachePath);
        return CMD_UNKNOWN;
    }
    write_log_contents(log_fp, log_contents);

    //free memory for dynamic allocation
    free(subCachePath);
    free(log_contents);
    printf("test1\n");
    return CMD_REPEAT;
}
