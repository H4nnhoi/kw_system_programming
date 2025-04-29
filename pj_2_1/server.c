#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/wait.h>

#define BUFFSIZE 1024
#define PORTNO 40000

static void handler() {
    pid_t pid;
    int status;
    while ((pid = waitpid(-1, &status, WNOHANG)) > 0);
}

int main() {
    struct sockaddr_in server_addr, client_addr;
    int socket_fd, client_fd;
    int len, len_out;
    char buf[BUFFSIZE];
    pid_t pid;

    if ((socket_fd = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
        printf("Server : Can't open stream socket\n");
        return 0;
    }

    bzero((char*)&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(PORTNO);

    if (bind(socket_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        printf("Server : Can't bind local address\n");
        close(socket_fd);
        return 0;
    }

    listen(socket_fd, 5);
    signal(SIGCHLD, (void *)handler);  // 자식 프로세스 종료 처리

    while (1) {
        bzero((char*)&client_addr, sizeof(client_addr));
        len = sizeof(client_addr);
        printf("test1\n");
        client_fd = accept(socket_fd, (struct sockaddr*)&client_addr, &len);

        if (client_fd < 0) {
            printf("Server : accept failed %d\n", getpid());
            close(socket_fd);
            return 0;
        }

        printf("[%d : %d] client was connected\n", client_addr.sin_addr.s_addr, client_addr.sin_port);
        pid = fork();

        if (pid == -1) {
            close(client_fd);
            close(socket_fd);
            continue;
        }

        if (pid == 0) {  // 자식 프로세스
            while ((len_out = read(client_fd, buf, BUFFSIZE)) > 0) {
                if (!strncmp(buf, "bye", 3)) {
                    break;
                }
                write(STDOUT_FILENO, " - Messages : ", 15);
                write(STDOUT_FILENO, buf, len_out);
                write(client_fd, buf, len_out);
                write(STDOUT_FILENO, "\n", 1);
            }

            printf("[%d : %d] client was disconnected\n", client_addr.sin_addr.s_addr, client_addr.sin_port);
            close(client_fd);
            exit(0);
        }
        close(client_fd);  // 부모 프로세스는 클라이언트 소켓 닫음
    }
    close(socket_fd);
    return 0;
}
