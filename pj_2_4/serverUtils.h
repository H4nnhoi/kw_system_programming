#ifndef SERVER_UTILS_H
#define SERVER_UTILS_H

#include <stddef.h> 

char* get_parsing_url(char* request);
int connect_to_webserver(const char *hostname, int port);
int send_http_request(int sockfd, const char* request);
int receive_http_response(int sockfd, char* buffer, size_t size);

#endif