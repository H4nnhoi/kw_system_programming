#include <stdio.h>        
#include <stdlib.h>      
#include <string.h>       
#include <unistd.h>      
#include <netdb.h>       
#include <arpa/inet.h>    
#include <netinet/in.h>   
#include <sys/socket.h>   
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

int connect_to_webserver(const char *hostname, int port) {
    struct sockaddr_in server_addr;
    struct hostent *host;
    int sock;

    if ((host = gethostbyname(hostname)) == NULL) {
        perror("gethostbyname failed");
        return -1;
    }

    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) return -1;

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    memcpy(&server_addr.sin_addr, host->h_addr, host->h_length);

    if (connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        close(sock);
        return -1;
    }

    return sock;
}

int send_http_request(int sockfd, const char* request) {
    size_t total_sent = 0;
    size_t request_len = strlen(request);
    
    while (total_sent < request_len) {
        ssize_t sent = write(sockfd, request + total_sent, request_len - total_sent);
        if (sent <= 0) {
            perror("write to web server failed");
            return -1;
        }
        total_sent += sent;
    }
    return 0;
}


int receive_http_response(int sockfd, char* buffer, size_t size) {
    ssize_t total_received = 0;
    ssize_t n;

    while ((n = read(sockfd, buffer + total_received, size - total_received)) > 0) {
        total_received += n;
        if (total_received >= size) {
            break;  // 버퍼 한계 도달
        }
    }

    if (n < 0) {
        perror("read from web server failed");
        return -1;
    }

    return total_received;
}
