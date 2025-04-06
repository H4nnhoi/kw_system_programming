#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <dirent.h>
#include "hit_and_miss.h"
#include "timeUtils.h"

const char* hit_format = "[Hit]%s-[%s]\n[Hit]%s\n";
const char* miss_format = "[Miss]%s-[%s]\n";
const char* termiante_format = "[Terminated] run time: %d sec. #request hit : %d, miss : %d\n";


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

char* get_miss_log(const char* url) {
    // time set
    char time_buf[64];
    get_formatted_time(time_buf, sizeof(time_buf));

    // contents set
    size_t needed = strlen(miss_format) - 4 +   // (%s * 2)
                    strlen(time_buf) +
                    strlen(url) + 1;           // count null
    char* result = malloc(needed);
    if (!result) return NULL;

    // get result of miss case
    snprintf(result, needed, miss_format, url, time_buf);
    return result;
}

char* get_hit_log(const char* hashed_path, const char* url) {
    // time set
    char time_buf[64];
    get_formatted_time(time_buf, sizeof(time_buf));

    // contents set
    size_t needed = strlen(hit_format) - 6 +   // (%s * 3)
                    strlen(hashed_path) +
                    strlen(time_buf) +
                    strlen(url) + 1;           // count null

    char* result = malloc(needed);
    if (!result) return NULL;

    // get result of hit case
    snprintf(result, needed, hit_format, hashed_path, time_buf, url);
    return result;
}
char* get_terminated_log(double process_sec, int hit_count, int miss_count){
    // convert type to int
    int seconds = (int)process_sec;

    // count only length (not save string)
    int num_len_sec  = snprintf(NULL, 0, "%d", seconds);
    int num_len_hit  = snprintf(NULL, 0, "%d", hit_count);
    int num_len_miss = snprintf(NULL, 0, "%d", miss_count);

    size_t needed = strlen(termiante_format) - 6 +   // (%d * 3)
                    num_len_sec +
                    num_len_hit +
                    num_len_miss + 1;           // count null

    char* result = malloc(needed);
    if (!result) return NULL;

    snprintf(result, needed, termiante_format, seconds, hit_count, miss_count);
    return result;
}