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
void createCacheFile(char *dirPath, char *fileName) {
    char fullPath[MAX_FULL_PATH_SIZE]; // full Path of file
    
    // 1. create file full path
    snprintf(fullPath, sizeof(fullPath), "%s/%s", dirPath, fileName);

    // // 2. find file
    // struct stat st;
    // if(stat(fullPath, &st) == 0) return 0;

    // 3.if not exist, create file
    FILE *fp = fopen(fullPath, "w");
    if (fp == NULL) {
        perror("fopen failed");
        return;
    }
    fclose(fp);
    return;

    // printf("Created cache file: %s\n", fullPath);
}
