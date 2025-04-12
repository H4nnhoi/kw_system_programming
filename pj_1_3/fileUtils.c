#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include "fileUtils.h"

#define MAX_FULL_PATH_SIZE 512

///////////////////////////////////////////////////////////////////////
// createCacheFile                                                   //
// ================================================================= //
// Input: char* dirPath   -> Directory path to create the file in    //
//        char* fileName  -> File name to create (no extension)      //
// Output: int 		1 if file was created			                 //
//			        0 if file already exists                         //
// 			        -1 if error occured		                         //
// Purpose:                                                          //
// Create an empty cache file with the given file name in            //
//          the specified directory. File name will have 	         //
//          extension and will be created as dirPath/fileName        //
///////////////////////////////////////////////////////////////////////
void createFile(const char *dirPath, const char *fileName) {
    char fullPath[MAX_FULL_PATH_SIZE]; // full Path of file
    
    // 1. create file full path
    snprintf(fullPath, sizeof(fullPath), "%s/%s", dirPath, fileName);


    // 2. create file
    // * edit : not consider exist, only write file
    FILE *fp = fopen(fullPath, "w");
    if (fp == NULL) {
        perror("fopen failed");
        return;
    }
    fclose(fp);
    return;

}


///////////////////////////////////////////////////////////////////////
// init_log                                                          //
// ================================================================= //
// Input: FILE** log_fp     -> Pointer to FILE* to initialize        //
//        const char* fullPath  -> Full path to log file             //
// Output: void                                                      //
// Purpose: Open the log file in append mode and assign to log_fp.   //
///////////////////////////////////////////////////////////////////////
void init_log(FILE **log_fp, const char* fullPath){
    *log_fp = fopen(fullPath, "a");
    if(*log_fp == NULL){
        perror("fopen failed");
        return;
    }
}
///////////////////////////////////////////////////////////////////////
// close_log                                                         //
// ================================================================= //
// Input: FILE** log_fp -> Pointer to the log file stream            //
// Output: void                                                      //
// Purpose: Safely close the opened log file if it exists.           //
///////////////////////////////////////////////////////////////////////
void close_log(FILE **log_fp) {
    if (*log_fp != NULL) fclose(*log_fp);
}

///////////////////////////////////////////////////////////////////////
// write_log_contents                                                //
// ================================================================= //
// Input: FILE** log_fp      -> Pointer to the log file stream       //
//        const char* contents    -> Log contents to write           //
// Output: void                                                      //
// Purpose: Write given log string into the log file immediately     //
//          using fflush to flush the stream.                        //
///////////////////////////////////////////////////////////////////////
void write_log_contents(FILE **log_fp, const char *contents){    
    if (*log_fp != NULL) {
        fprintf(*log_fp, "%s", contents);
        fflush(*log_fp);         //immediately write
    }

}
