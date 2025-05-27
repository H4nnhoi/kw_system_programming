#ifndef SUB_PROCESS_H
#define SUB_PROCESS_H

#include <sys/types.h>
#include <stdio.h>

char *getIPAddr(char *addr);
int sub_process(char *inputURL, FILE *log_fp, const char *cachePath, int client_fd);

#endif
