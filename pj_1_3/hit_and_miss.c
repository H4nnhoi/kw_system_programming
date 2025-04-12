#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <dirent.h>
#include "hit_and_miss.h"
#include "timeUtils.h"

const char* hit_format = "[Hit]%s-[%s]\n[Hit]%s\n";
const char* miss_format = "[Miss]%s-[%s]\n";
const char* termiante_format = "[Terminated] run time: %d sec. #request hit : %d, miss : %d\n";
const char* server_termiante_format = "**SERVER** [Terminated] run time: %d sec. #sub process : %d\n";

///////////////////////////////////////////////////////////////////////
// is_file_hit                                                       //
// ================================================================= //
// Input: const char* dir  -> Directory path to search in            //
//        const char* file -> File name to search for                //
// Output: int                                                       //
//          0 -> File does not exist (MISS)                          //
//          1 -> File exists (HIT)                                   //
// Purpose: Use opendir/readdir to determine if a specific file      //
//          exists in the given directory.                           //
///////////////////////////////////////////////////////////////////////
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

///////////////////////////////////////////////////////////////////////
// get_miss_log                                                      //
// ================================================================= //
// Input: const char* url -> Original URL input                      //
// Output: char*                                                     //
//         Dynamically allocated log string for MISS case            //
// Purpose: Create formatted log string for MISS case including      //
//          URL and timestamp.                                       //
///////////////////////////////////////////////////////////////////////
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

///////////////////////////////////////////////////////////////////////
// get_hit_log                                                       //
// ================================================================= //
// Input: const char* hashed_path -> Hashed path string              //
//        const char* url         -> Original URL input              //
// Output: char*                                                     //
//         Dynamically allocated log string for HIT case             //
// Purpose: Create formatted log string for HIT case including       //
//          hashed path, time, and original URL.                     //
///////////////////////////////////////////////////////////////////////
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

///////////////////////////////////////////////////////////////////////
// get_terminated_log                                                //
// ================================================================= //
// Input: double process_sec -> Total execution time in seconds      //
//        int hit_count      -> Total HIT count                      //
//        int miss_count     -> Total MISS count                     //
// Output: char*                                                     //
//         Dynamically allocated termination log string              //
// Purpose: Return a formatted string summarizing execution time     //
//          and hit/miss statistics.                                 //
///////////////////////////////////////////////////////////////////////
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

char* get_server_terminated_log(double process_sec, int sub_process_count){
    // convert type to int
    int seconds = (int)process_sec;

    // count only length (not save string)
    int num_len_sec  = snprintf(NULL, 0, "%d", seconds);
    int num_len_process  = snprintf(NULL, 0, "%d", sub_process_count);

    size_t needed = strlen(server_termiante_format) - 4 +   // (%d * 2)
                    num_len_sec +
                    num_len_process + 1;           // count null

    char* result = malloc(needed);
    if (!result) return NULL;

    snprintf(result, needed, server_termiante_format, seconds, sub_process_count);
    return result;
}
