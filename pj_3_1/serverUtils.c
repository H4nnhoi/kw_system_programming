#include <stdio.h>        
#include <stdlib.h>      
#include <string.h>       
#include <unistd.h>      
#include <netdb.h>       
#include <arpa/inet.h>    
#include <netinet/in.h>   
#include <sys/socket.h>   
#define BUFFSIZE 1024
#define READ_BLOCK_SIZE 4096


///////////////////////////////////////////////////////////////////////
// get_parsing_url                                                   //
///////////////////////////////////////////////////////////////////////
// Input:                                                            //
//   - const char* request : Raw HTTP request string                 //
// Return:                                                           //
//   - char* : Extracted URL from  request                           //
// Description:                                                      //
//   - Parses the HTTP request and extracts the requested URL        //
///////////////////////////////////////////////////////////////////////
char* get_parsing_url(const char* request){
    char tmp[BUFFSIZE] = {0, };
    char url[BUFFSIZE] = {0, };
    char method[20] = {0, };
    char *tok = NULL;

    strcpy(tmp, request);
    // HTTP 요청 파싱
    tok = strtok(tmp, " ");
    strcpy(method, tok);
    tok = strtok(NULL, " ");
    strcpy(url, tok);
    
    
    return strdup(url);
}

///////////////////////////////////////////////////////////////////////
// get_parsing_host_and_path                                         //
///////////////////////////////////////////////////////////////////////
// Input:                                                            //
//   - const char* url : The full URL string (e.g., "http://...")    //
//   - char* host      : Output buffer to store the parsed hostname  //
//   - char* path      : Output buffer to store the parsed path      //
// Output:                                                           //
//   - None (results are written into host and path arguments)       //
// Purpose:                                                          //
//   - Parses a full URL into its hostname and path components       //
//   - Removes the "http://" prefix and separates host and path      //
//   - If no path is provided, defaults to "/"                       //
///////////////////////////////////////////////////////////////////////
void get_parsing_host_and_path(const char* url, char *host, char *path){
	char temp[BUFFSIZE];
	strcpy(temp, url);

	char *token;

	if(strncmp(temp, "http://", 7) == 0){
		token = temp + 7;
	} else {
		perror("not valid url");
	}

	char *slash = strchr(token, '/');
	if(slash){
		size_t host_len = slash - token;
		strncpy(host, token, host_len);
		host[host_len] = '\0';

		snprintf(path, 512, "%s", slash);
	} else{
		strcpy(host, token);
		strcpy(path, "/");
	}
}

///////////////////////////////////////////////////////////////////////
// connect_to_webserver                                              //
///////////////////////////////////////////////////////////////////////
// Input:                                                            //
//   - const char* hostname : Domain name of the target server       //
//   - int port             : Port number (e.g., 80 for HTTP)        //
// Return:                                                           //
//   - int : Connected socket descriptor on success, -1 on failure   //
// Description:                                                      //
//   - Resolves the hostname to an IP address                        //
//   - Creates a TCP socket and connects to the target web server    //
///////////////////////////////////////////////////////////////////////
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
///////////////////////////////////////////////////////////////////////
// send_http_request                                                 //
///////////////////////////////////////////////////////////////////////
// Input:                                                            //
//   - int sockfd         : Socket file descriptor                   //
//   - const char* request : HTTP request string to send             //
// Return:                                                           //
//   - int : 0 on success, -1 on failure                             //
// Description:                                                      //
//   - Sends the full HTTP request to the connected web server       //
//   - Handles partial sends by looping until all data is sent       //
///////////////////////////////////////////////////////////////////////
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

////////////////////////////////////////////////////////////////////////
// receive_http_response                                              //
////////////////////////////////////////////////////////////////////////
// Input:                                                             //
//   - int sockfd       : Socket file descriptor connected to a host //
//   - size_t* out_size : Pointer to store the total size of the data //
// Output:                                                            //
//   - char*            : Pointer to a dynamically allocated buffer   //
//                        containing the full HTTP response           //
//                        (must be freed by the caller)               //
//                        Returns NULL on failure                     //
// Purpose:                                                           //
//   - Reads a full HTTP response from the connected socket           //
//   - Dynamically expands the buffer as data is received             //
//   - Parses the Content-Length header to determine full message size//
//   - Ensures the response is null-terminated for string safety      //
////////////////////////////////////////////////////////////////////////
char* receive_http_response(int sockfd, size_t *out_size) {
    char *buffer = NULL;
    size_t buffer_size = 0;
    size_t total_received = 0;
    size_t content_length = 0;
    int header_parsed = 0;
    char *header_end = NULL;
    

    while (1) {
        //if want to expand size, re-allocate
        buffer = realloc(buffer, buffer_size + READ_BLOCK_SIZE + 1);
        if(buffer == NULL){
            perror("realloc failed");
            return NULL;
        }
	// get response message
        size_t n = read(sockfd, buffer + total_received, READ_BLOCK_SIZE);
        if(n < 0){
            perror("read failed");
            free(buffer);
            return NULL;
        }
        else if(n == 0){
            if(header_parsed && total_received >= content_length) {
		    break;
	    }
	    else{
		    usleep(5000);
		    continue;
	    }
        }

        total_received += n;
	buffer_size += n;
	buffer[total_received] = '\0';

	// if not found header yet
	if(!header_parsed) {
		header_end = strstr(buffer, "\r\n\r\n");
		if(!header_end) continue;

		header_parsed = 1;

		// find Content-Length
		char *cl_ptr = strstr(buffer, "Content-Length:");
		if(cl_ptr == NULL){
			perror("Content Length not found");
			return NULL;
		}
		// move offset
		cl_ptr += strlen("Content-Length:");
		while (*cl_ptr == ' ') cl_ptr++;
		content_length = atoi(cl_ptr);

		int header_size = (header_end - buffer) + 4;
		content_length += header_size;
	}
	
	if(header_parsed && total_received >= content_length){
		break;
	}
    }

    if(out_size != NULL){
	    *out_size = total_received;
    }
    return buffer;
}

