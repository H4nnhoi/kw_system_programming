#include <stdio.h>
#include <string.h>
#include "sha1_utils.h"
#include "dirUtils.h"
#include "fileUtils.h"

#define MAX_INPUT 256
#define CACHE_DIR_SIZE 4
#define FILE_SIZE 40
char home[MAX_INPUT];
char cachePath[MAX_INPUT];

int main() {
    // value setting(empty input, home path, cache path)
    char input[MAX_INPUT];
    getHomeDir(home);
    snprintf(cachePath, MAX_INPUT, "%s/cache", home);
    // if cache directory didnt exsit, make directory permit 777
    ensureDirExist(cachePath, 0777);

    while (1) {
        char hashed_url[41];
        printf("input url> ");
        fflush(stdout);  // 출력 버퍼 비우기 

        if (fgets(input, sizeof(input), stdin) == NULL) {       //insert url
            printf("error from insert\n");
            break;                                              // break when error from insert url
        }

        // 개행 문자 제거
        input[strcspn(input, "\n")] = '\0';

        if (strcmp(input, "bye") == 0) {        // if insert is bye
            printf("Goodbye!\n");
            break;                              // over the program
        }
        // get hashed URL using sha1_hash function
        sha1_hash(input, hashed_url);
        printf("result hashed_url = %s\n", hashed_url);

        // divide hashed_url
        char subdir[CACHE_DIR_SIZE];
        strncpy(subdir, hashed_url, 3);
        subdir[3] = '\0';

        // create directory by divide hashed_url
        char subCachePath[MAX_INPUT];
        snprintf(subCachePath, MAX_INPUT, "%s/%s", cachePath, subdir);
        ensureDirExist(subCachePath, 0777);

        // create file by divide hashed_url
        char fileName[FILE_SIZE];
        strncpy(fileName, hashed_url + 2, sizeof(fileName) - 1);
        fileName[sizeof(fileName) - 1] = '\0'; 

        createCacheFile(cachePath, hashed_url, sizeof(hashed_url) - sizeof(subdir));
        

    }

    return 0;
}