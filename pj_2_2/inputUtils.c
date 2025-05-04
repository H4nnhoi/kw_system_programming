#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define MAX_INPUT 256


///////////////////////////////////////////////////////////////////////
// get_input_url                                                     //
// ================================================================= //
// Input: int init_buf_size -> Initial buffer size for input string  //
// Output: char* -> Dynamically allocated user input string          //
// Purpose: Reads user input (until newline) dynamically by          //
//          expanding buffer size as needed using realloc().         //
//          Returns a malloc'ed string, Caller must free it.         //
///////////////////////////////////////////////////////////////////////
// * edit input to Dynamic Memory Allocation (not static allocation)
char* get_input(int init_buf_size){
    
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
///////////////////////////////////////////////////////////////////////
// make_dir_path                                                     //
// ================================================================= //
// Input: const char* base_path -> Base directory path               //
//        const char* subdir    -> Subdirectory or filename to append//
// Output: char* -> Full path string in format "base_path/subdir"    //
// Purpose: Concatenate base_path and subdir with '/' separator and  //
//          return newly allocated string. Caller must free it.      //
///////////////////////////////////////////////////////////////////////
char* make_dir_path(const char* base_path, const char* subdir) {
    size_t needed_size = strlen(base_path) + strlen(subdir) + 2; // "/" + '\0'
    char* full_path = malloc(needed_size);
    if (!full_path) return NULL;

    snprintf(full_path, needed_size, "%s/%s", base_path, subdir);
    return full_path;
}

///////////////////////////////////////////////////////////////////////
// compare_input_cmd                                                 //
// ================================================================= //
// Input : char* cmd  -> User input command string                   //
// Output: int        -> Parsed result                               //
//                      1 : "connect" command                        //
//                      0 : "quit" command                           //
//                     -1 : Unknown or invalid command               //
// Purpose: Determine the type of user command input and return      //
//          a corresponding integer code to control program flow.    //
///////////////////////////////////////////////////////////////////////
int compare_input_cmd(char* cmd){
    
    if (strcmp(cmd, "connect") == 0) {      // CONNECT
        return 1;
    } else if (strcmp(cmd, "quit") == 0) {  // QUIT
        return 0;
    } 
    
    return -1;          //exception

}