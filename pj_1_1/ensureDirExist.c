#include <sys/stat.h>
#include <sys/types.h>
#include <stdio.h>

mode_t old_umask;

void ensureDirExist(char *cachePath, int permission_value){
    struct stat st;
    old_umask = umask(0);


    // 1. check exist file
    // 2. if not exist, make file
    // 3. (exception) if cannot make file, perror
    if (stat(cachePath, &st) == -1 && mkdir(cachePath, permission_value) != 0)  {
        perror("mkdir failed");
    } 
}