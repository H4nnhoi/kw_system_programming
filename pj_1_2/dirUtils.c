#include <sys/stat.h>
#include <sys/types.h>
#include <stdio.h>
#include <unistd.h>
#include <pwd.h>
#include <string.h>
#include "dirUtils.h"

mode_t old_umask;

///////////////////////////////////////////////////////////////////////
// ensureDirExist                                                    //
// ================================================================= //
// Input: char* cachePath      -> Directory path to check/create     //
//        int permission_value -> Permission value to apply          //
//                                                    (e.g. 0777)    //
// Output: void                                                      //
// Purpose: Check if the given directory exists. If not, create it   //
//          with the specified permissions.                          //
///////////////////////////////////////////////////////////////////////
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


///////////////////////////////////////////////////////////////////////
// getHomeDir                                                        //
// ================================================================= //
// Input: char* home -> Buffer to store the home directory path      //
// Output: char*     -> Pointer to the home buffer                   //
// Purpose: Retrieve the current user's home directory and store it  //
//          in the provided buffer.                                  //
///////////////////////////////////////////////////////////////////////
char *getHomeDir(char *home){
    struct passwd *usr_info = getpwuid(getuid());
    strcpy(home, usr_info->pw_dir);

    return home;
}