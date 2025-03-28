#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include "sha1_utils.h"
#include "home_dir_utils.h"

#define MAX_INPUT 256
mode_t old_umask;
char home[MAX_INPUT];
char cachePath[MAX_INPUT];

int main() {
    // value setting(empty input, home path, cache path)
    char input[MAX_INPUT];
    getHomeDir(home);
    snprintf(cachePath, MAX_INPUT, "%s/cache", home);
    old_umask = umask(0);
    struct stat st;
    

    if (stat(cachePath, &st) == -1) {
        // 디렉토리가 없으면 생성
        if (mkdir(cachePath, 0777) == 0) {
            printf("Created cache directory at: %s\n", cachePath);
        } else {
            perror("mkdir failed");
        }
    } else {
        // 이미 존재하는 경우
        printf("Cache directory exists at: %s\n", cachePath);
    }


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



    }

    return 0;
}