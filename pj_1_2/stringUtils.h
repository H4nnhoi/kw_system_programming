#ifndef STRING_UTILS_H
#define STRING_UTILS_H

#include <stddef.h> 

char* get_input_url(int init_buf_size);
char* make_dir_path(const char* base_path, const char* subdir);

#endif