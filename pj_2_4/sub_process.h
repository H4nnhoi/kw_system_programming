#ifndef SUB_PROCESS_H
#define SUB_PROCESS_H

#include <sys/types.h>
#include <stdio.h>

char *getIPAddr(char *addr);
int sub_process(char *inputURL, pid_t* PID, FILE *log_fp, const char *cachePath, time_t sub_start_time, int *hit_count, int *miss_count, char* buf, size_t buf_size);

#endif