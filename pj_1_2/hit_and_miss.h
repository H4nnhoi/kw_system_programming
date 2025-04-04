#ifndef HIT_AND_MISS_H
#define HIT_AND_MISS_H
int is_file_hit(const char* dir, const char* file);
void get_miss_log(const char* url, char* buffer, size_t size);
void get_hit_log(const char* hashed_path, const char* url, char* buffer, size_t size);
#endif