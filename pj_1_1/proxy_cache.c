#include <stdio.h>
#include <string.h>
#include "sha1_utils.h"

#define MAX_INPUT 256

int main() {
    char input[MAX_INPUT];

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
        sha1_hash(input, hashed_url);
        printf("result hashed_url = %s\n", hashed_url);

    }

    return 0;
}