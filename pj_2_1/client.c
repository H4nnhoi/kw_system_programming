#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include "inputUtils.h"

#define BUFFSIZE 1024
#define PORTNO 40000

int main() {
    int socket_fd, len;
    struct sockaddr_in server_addr;
    char hadddr[] = "127.0.0.1";  // 로컬 호스트 IP 주소
    char buf[BUFFSIZE];

    // 소켓 생성
    if ((socket_fd = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
        printf("can't create socket.\n");
        return -1;
    }

    // 버퍼 및 서버 주소 초기화
    bzero(buf, sizeof(buf));
    bzero((char *)&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(hadddr);
    server_addr.sin_port = htons(PORTNO);

    // 서버에 연결
    if (connect(socket_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        printf("can't connect.\n");
        return -1;
    }

    while(1){
        printf("input URL> ");
        char *sendURL = get_input(BUFFSIZE);
        len = strlen(sendURL) + 1;
        send(socket_fd, &len, sizeof(int), 0);
        send(socket_fd, sendURL, len, 0);
        if(strcmp(sendURL, "bye") == 0){
            free(sendURL);
            break;
        }
        free(sendURL);
    }

    close(socket_fd);
    return 0;
}
