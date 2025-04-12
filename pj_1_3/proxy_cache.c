///////////////////////////////////////////////////////////////////////
// File Name : Main.c                                                //
// Date : 2025/03/30                                                 //
// Os : Ubuntu 16.04 LTS 64bits                                      //
// Author : Lee Jeong Han                                            //
// Student ID : 2020202047                                           //
// ----------------------------------------------------------------- //
// Title : System Programming Assignment #1-2 (proxy server)         //
// Description : cache program hashes user-input URLs                //
//               using SHA1 and stores them as files                 //
//               in a structured cache directory based on the hash.  //
//               Additionally, it logs cache HIT or MISS results     //
//               to a logfile depending on outcome.                  //
///////////////////////////////////////////////////////////////////////
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

///////////////////////////////////////////////////////////////////////
// vars_setting                                                      //
// ================================================================= //
// Input : void                                                      //
// Output: void                                                      //
// Purpose: Perform all initial setup before the main loop begins.   //
//          - INIT start time for execution duration calculation     //
//          - INIT hit/miss counters                                 //
//          - Build paths for cache and log directories              //
//          - Ensure cache and log directories exist (0777 perms)    //
//          - Create logfile.txt if it does not exist                //
//          - Open logfile in append mode using init_log()           //
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
int child_process(){
    // setting sub_process
    char hashed_url[41];
    char subdir[CACHE_DIR_SIZE];
    char fileName[FILE_SIZE];
    char* input = NULL;
    char* log_contents = NULL;
    char* subCachePath = NULL;
    
    printf("[%d]input URL> ", getpid());
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
        printf("last log is %s\n", terminate_log);
        write_log_contents(&log_fp, terminate_log);
        free(terminate_log);
        free(input);
        return CMD_EXIT;
    }
        
     // get hashed URL using sha1_hash function
    sha1_hash(input, hashed_url);
    // printf("result hashed_url = %s\n", hashed_url);

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
    // printf("log contents = %s\n", log_contents);
    write_log_contents(&log_fp, log_contents);

    //free memory for dynamic allocation
    free(input);                    
    free(subCachePath);
    free(log_contents);

    return CMD_REPEAT;
}



int main() {
    vars_setting();
    

    while (1) {
        printf("[%d]input CMD> ", getpid());
        char* input_cmd = get_input(MAX_INPUT);
        int cmd_result = compare_input_cmd(input_cmd);

        if (cmd_result == CMD_REPEAT) {
            // PROCESS START
            PID = fork();
            if (PID < 0) {
                perror("fork failed");
                continue;
            }

            if (PID == 0) {
                // Child process
                time(&sub_start_time);
                while (1) {
                    int process_result = child_process();

                    if (process_result == CMD_EXIT) break;
                    if (process_result == CMD_REPEAT) continue;
                    //EXCEPTION
                    fprintf(stderr, "Unknown process result\n");
                    break;
                }
                exit(0); // exit child process
            } else {
                // Parent process: wait for all childs process exited
                sub_process_count++;        //*** point ***
                waitpid(PID, NULL, 0);
            }

        } else if (cmd_result == CMD_EXIT) {
            // PROCESS END
            time(&end_time);
            char* server_terminated_log = get_server_terminated_log(difftime(end_time, start_time), sub_process_count);
            write_log_contents(&log_fp, server_terminated_log);
            free(server_terminated_log);
            close_log(&log_fp);
            break;
        } else {
            perror("Bad command input");
        }
        free(input_cmd);
        
    }
    
    return 0;
}
