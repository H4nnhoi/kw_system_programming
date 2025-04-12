#ifndef HIT_AND_MISS_H
#define HIT_AND_MISS_H
int is_file_hit(const char* dir, const char* file);
char* get_miss_log(const char* url);
char* get_hit_log(const char* hashed_path, const char* url);
char* get_terminated_log(double process_sec, int hit_count, int miss_count);
char* get_server_terminated_log(double process_sec, int sub_process_count);
#endif