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
pid_t PID;


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
// child_process                                                     //
// ================================================================= //
// Input : void                                                      //
// Output: int CMD_EXIT / CMD_REPEAT / CMD_UNKNOWN                   //
// Purpose: Handles one cycle of URL input, SHA1 hashing, cache      //
//          checking, file writing, and HIT/MISS logging.            //
//          When user inputs "bye", process ends and termination     //
//          log is generated.                                        //
///////////////////////////////////////////////////////////////////////
int sub_process(pid_t* PID){
    vars_setting();
    // setting sub_process
    char hashed_url[41];
    char subdir[CACHE_DIR_SIZE];
    char fileName[FILE_SIZE];
    char* input = NULL;
    char* log_contents = NULL;
    char* subCachePath = NULL;
    while(1){
        printf("input URL> ");
        input = get_input(MAX_INPUT);

        if (input == NULL) {
            perror("input is null");
            return CMD_UNKNOWN;
        }
            
        // end cmd
        if (strcmp(input, "bye") == 0) {
            time_t sub_end_time;
            time(&sub_end_time);
            char* terminate_log = get_terminated_log(difftime(sub_end_time, sub_start_time), hit_count, miss_count);
            write_log_contents(&log_fp, terminate_log);
            free(terminate_log);
            free(input);
            free(&log_fp);
            return CMD_EXIT;
        }
            
        // get hashed URL using sha1_hash function
        sha1_hash(input, hashed_url);

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
            log_contents = get_miss_log(input);
            ensureDirExist(subCachePath, 0777);
            createFile(subCachePath, fileName);
            miss_count++;
        }else if(hit_and_miss_result == 1){      // HIT
            char* hashed_path = make_dir_path(subdir, fileName);
            log_contents = get_hit_log(hashed_path, input);
            free(hashed_path);
            hit_count++;
        }else{
            perror("not range of return\n");
            free(input);
            free(subCachePath);
            return CMD_UNKNOWN;
        }
        write_log_contents(&log_fp, log_contents);

        //free memory for dynamic allocation
        free(input);                    
        free(subCachePath);
        free(log_contents);
    }
    return CMD_UNKNOWN;
    
    
}
