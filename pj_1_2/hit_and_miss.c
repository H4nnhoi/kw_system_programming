#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include "hit_and_miss.h"


int is_cache_hit(const char* dir, const char* file) {
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