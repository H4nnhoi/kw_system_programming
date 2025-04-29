#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

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

    // 통신 시작
    write(STDOUT_FILENO, "> ", 2);
    while ((len = read(STDIN_FILENO, buf, sizeof(buf))) > 0) {
        if (write(socket_fd, buf, strlen(buf)) > 0) {
            if ((len = read(socket_fd, buf, sizeof(buf))) > 0) {
                write(STDOUT_FILENO, buf, len);
                bzero(buf, sizeof(buf));
            }
        }
        write(STDOUT_FILENO, "> ", 2);
    }

    close(socket_fd);
    return 0;
}
