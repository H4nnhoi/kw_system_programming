#include <stdio.h>
#include <string.h>
#include <time.h>

void get_formatted_time(char* buffer, size_t size) {
    time_t now = time(NULL);
    struct tm* t = localtime(&now);
    strftime(buffer, size, "%Y/%m/%d, %H:%M:%S", t);
}