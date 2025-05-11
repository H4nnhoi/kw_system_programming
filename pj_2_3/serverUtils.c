#include <stdio.h>
#include <string.h>
#define BUFFSIZE 1024

char* get_parsing_url(char* request){
    char tmp[BUFFSIZE] = {0, };
    char url[BUFFSIZE] = {0, };
    char method[20] = {0, };
    char *tok = NULL;

    strcpy(tmp, request);
    // HTTP 요청 파싱
    tok = strtok(tmp, " ");
    strcpy(method, tok);
    if (strcmp(method, "GET") == 0){
        tok = strtok(NULL, " ");
        strcpy(url, tok);
    }else{
        perror("method type wrong");
        exit(0);
    }
}