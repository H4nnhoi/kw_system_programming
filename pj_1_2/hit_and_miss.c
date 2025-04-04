#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include "hit_and_miss.h"
#include "timeUtils.h"


int is_file_hit(const char* dir, const char* file) {
    DIR* dp = opendir(dir);
    if (dp == NULL) {
        // if not exist dir, return miss
        return 0;
    }

    struct dirent* entry;
    while ((entry = readdir(dp)) != NULL) {
        if (strcmp(entry->d_name, file) == 0) {
            closedir(dp);
            return 1; // HIT
        }
    }

    closedir(dp);
    return 0; // MISS
}

void get_miss_log(const char* url, char* buffer, size_t size) {
    char time_buf[64];
    get_formatted_time(time_buf, sizeof(time_buf));
    snprintf(buffer, size, "[Miss]%s-[%s]\n", url, time_buf);
}

void get_hit_log(const char* hashed_path, const char* url, char* buffer, size_t size) {
    char time_buf[64];
    get_formatted_time(time_buf, sizeof(time_buf));
    snprintf(buffer, size,
             "[Hit]%s-[%s]\n"
             "[Hit]%s\n", hashed_path, time_buf, url);
}