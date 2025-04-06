///////////////////////////////////////////////////////////////////////
// File Name : Main.c                                                //
// Date : 2025/03/30                                                 //
// Os : Ubuntu 16.04 LTS 64bits                                      //
// Author : Lee Jeong Han                                            //
// Student ID : 2020202047                                           //
// ----------------------------------------------------------------- //
// Title : System Programming Assignment #1-1 (proxy server)         //
// Description : A simple C program that hashes user-input URLs      //
// using SHA1 and stores them as files                               // 
// in a structured cache directory based on the hash.                //
///////////////////////////////////////////////////////////////////////
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "sha1Utils.h"
#include "dirUtils.h"
#include "fileUtils.h"
#include "hit_and_miss.h"

#define MAX_INPUT 256
#define CACHE_DIR_SIZE 4
#define FILE_SIZE 40
char home[MAX_INPUT];
char cachePath[MAX_INPUT];
char logPath[MAX_INPUT];
char logFile[MAX_INPUT];
FILE *log_fp;

void vars_setting(){
    // value setting(empty input, home path, cache path)
    
    getHomeDir(home);
    snprintf(cachePath, MAX_INPUT, "%s/cache", home);
    snprintf(logPath, MAX_INPUT, "%s/logfile", home);
    // if cache&log directory didnt exsit, make directory permit 777
    ensureDirExist(cachePath, 0777);
    ensureDirExist(logPath, 0777);
    char logfileName[MAX_INPUT] = "logfile.txt";
    // check logfile.txt exist and make file
    if(is_file_hit(logPath, logfileName) == 0){
        createFile(logPath, logfileName);
    }
    //open file
    char log_full_path[MAX_INPUT];
    snprintf(log_full_path, MAX_INPUT, "%s/%s", logPath, logfileName);
    init_log(&log_fp, log_full_path);
}

// * edit input to Dynamic Memory Allocation (not static allocation)
char* get_input_url(){
    
    printf("input url> ");
    size_t bufsize = MAX_INPUT;
    char* buffer = malloc(bufsize);
    if (!buffer) return NULL;

    size_t len = 0;
    int ch;

    // loop while meet newline & EOF
    // using getchar() method for input url
    while ((ch = getchar()) != '\n' && ch != EOF) {
        // need more bufsize, add it
        if (len + 1 >= bufsize) {
            bufsize *= 2;
            char* new_buf = realloc(buffer, bufsize);
            if (!new_buf) {
                free(buffer);
                return NULL;
            }
            buffer = new_buf;
        }
        buffer[len++] = (char)ch;
        buffer[len] = '\0';
    }

    return buffer;
}



int main() {
    vars_setting();
    

    while (1) {
        char hashed_url[41];
        char* input = get_input_url();
        printf("input value = %s", input);
        
        // get hashed URL using sha1_hash function
        sha1_hash(input, hashed_url);
        // printf("result hashed_url = %s\n", hashed_url);

        // divide hashed_url
        char subdir[CACHE_DIR_SIZE];
        strncpy(subdir, hashed_url, 3);
        subdir[3] = '\0';

        // create file by divide hashed_url
        // * edit file path range [3-40]
        char fileName[FILE_SIZE];
        strncpy(fileName, hashed_url + 3, sizeof(fileName) - 1);
        fileName[sizeof(fileName) - 1] = '\0'; 

        char subCachePath[MAX_INPUT];
        snprintf(subCachePath, MAX_INPUT, "%s/%s", cachePath, subdir);

        int hit_and_miss_result = is_file_hit(subCachePath, fileName);
        char log_contents[MAX_INPUT];

        // HIT & MISS case
        if(hit_and_miss_result < 0){
            perror("error from cache\n");

        }else if(hit_and_miss_result == 0){      // MISS
            get_miss_log(input, log_contents, sizeof(log_contents));
            ensureDirExist(subCachePath, 0777);
            createFile(subCachePath, fileName);
        }else if(hit_and_miss_result == 1){      // HIT
            char hashed_path[MAX_INPUT];
            snprintf(hashed_path, MAX_INPUT, "%s/%s", subdir, fileName);
            get_hit_log(hashed_path, input, log_contents, sizeof(log_contents));
            //TODO write logfile
        }else{
            perror("not range of return\n");
        }
        write_log_contents(&log_fp, log_contents);
    }

    return 0;
}
