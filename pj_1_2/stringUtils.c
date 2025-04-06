#include <stdlib.h>
#include <stdio.h>
#include <string.h>

// * edit input to Dynamic Memory Allocation (not static allocation)
char* get_input_url(int init_buf_size){
    
    printf("input url> ");
    size_t bufsize = init_buf_size;
    char* buffer = malloc(bufsize);
    if (!buffer) return NULL;

    size_t len = 0;
    int ch;

    // loop while meet newline & EOF
    // using getchar() method for input url by user
    while ((ch = getchar()) != '\n' && ch != EOF) {
        // need more bufsize, add it
        if (len + 1 >= bufsize) {
            bufsize *= 2;
            char* new_buf = realloc(buffer, bufsize);
            if (!new_buf) {
                free(buffer);
                return NULL;
            }
            buffer = new_buf;
        }
        buffer[len++] = (char)ch;
        buffer[len] = '\0';
    }

    return buffer;
}

char* make_dir_path(const char* base_path, const char* subdir) {
    size_t needed_size = strlen(base_path) + strlen(subdir) + 2; // "/" + '\0'
    char* full_path = malloc(needed_size);
    if (!full_path) return NULL;

    snprintf(full_path, needed_size, "%s/%s", base_path, subdir);
    return full_path;
}