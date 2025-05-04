#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#define BUFFSIZE 1024
#define PORTNO 40000

int main()
{
    struct sockaddr_in server_addr, client_addr;
    int socket_fd, client_fd;
    int len, len_out;
    char buf[BUFFSIZE];

    if ((socket_fd = socket(PF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("Server : Can't open stream socket\n");
        return 0;
    }

    bzero((char *)&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(PORTNO);

    if (bind(socket_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        printf("Server : Can't bind local address\n");
        return 0;
    }

    listen(socket_fd, 5);

    while (1)
    {
        struct in_addr inet_client_address;

        char response_header[BUFFSIZE] = {0, };
        char response_message[BUFFSIZE] = {0, };

        char tmp[BUFFSIZE] = {0, };
        char method[20] = {0, };
        char url[BUFFSIZE] = {0, };

        char *tok = NULL;

        len = sizeof(client_addr);
        client_fd = accept(socket_fd, (struct sockaddr*)&client_addr, &len);
        if (client_fd < 0)
        {
            printf("Server : accept failed\n");
            return 0;
        }

        inet_client_address.s_addr = client_addr.sin_addr.s_addr;

        printf("[%s : %d] client was connected\n", inet_ntoa(inet_client_address), client_addr.sin_port);

        read(client_fd, buf, BUFFSIZE);
        strcpy(tmp, buf);

        puts("==============================================");
        printf("Request from [%s : %d]\n", inet_ntoa(inet_client_address), client_addr.sin_port);
        puts(buf);
        puts("==============================================");

        // HTTP 요청 파싱
        tok = strtok(tmp, " ");
        strcpy(method, tok);
        if (strcmp(method, "GET") == 0)
        {
            tok = strtok(NULL, " ");
            strcpy(url, tok);
        }

        // 응답 본문
        sprintf(response_message,
            "<h1>RESPONSE</h1><br>"
            "Hello %s:%d<br>"
            "%s", inet_ntoa(inet_client_address), client_addr.sin_port, url);

        // HTTP 헤더 작성
        sprintf(response_header,
            "HTTP/1.0 200 OK\r\n"
            "Server:2018 simple web server\r\n"
            "Content-length:%lu\r\n"
            "Content-type:text/html\r\n\r\n",
            strlen(response_message));

        // 전송
        write(client_fd, response_header, strlen(response_header));
        write(client_fd, response_message, strlen(response_message));

        printf("[%s : %d] client was disconnected\n", inet_ntoa(inet_client_address), client_addr.sin_port);
        close(client_fd);
    }

    close(socket_fd);
    return 0;
}
