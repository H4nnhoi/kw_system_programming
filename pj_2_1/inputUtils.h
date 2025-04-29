#ifndef INPUT_UTILS_H
#define INPUT_UTILS_H

#include <stddef.h> 

char* get_input(int init_buf_size);
char* make_dir_path(const char* base_path, const char* subdir);
int compare_input_cmd(char* cmd);

#endif