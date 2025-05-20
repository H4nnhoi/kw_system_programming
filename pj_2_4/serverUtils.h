#ifndef SERVER_UTILS_H
#define SERVER_UTILS_H

#include <stddef.h> 

char* get_parsing_url(char* request);
void get_parsing_host_and_path(const char* url, char *host, char *path);
int connect_to_webserver(const char *hostname, int port);
int send_http_request(int sockfd, const char* request);
char* receive_http_response(int sockfd, size_t *out_size);

#endif
