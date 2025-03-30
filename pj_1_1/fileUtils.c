#include <stdio.h>
#include <string.h>
#include "fileUtils.h"

#define MAX_FULL_PATH_SIZE 512

///////////////////////////////////////////////////////////////////////
// createCacheFile                                                   //
// ================================================================= //
// Input: char* dirPath   -> Directory path to create the file in    //
//        char* fileName  -> File name to create (no extension)      //
// Output: void                                                      //
// Purpose:                                                          //
// Create an empty cache file with the given file name in            //
//          the specified directory. File name will have ".txt"      //
//          extension and will be created as dirPath/fileName.txt.   //
///////////////////////////////////////////////////////////////////////
void createCacheFile(char *dirPath, char *fileName) {
    char fullPath[MAX_FULL_PATH_SIZE]; // full Path of file
    
    // 1. create file full path
    snprintf(fullPath, sizeof(fullPath), "%s/%s.txt", dirPath, fileName);

    // 2. create file
    FILE *fp = fopen(fullPath, "w");
    if (fp == NULL) {
        perror("fopen failed");
        return;
    }
    fclose(fp);

    // printf("Created cache file: %s\n", fullPath);
}