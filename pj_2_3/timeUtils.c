#include <stdio.h>
#include <string.h>
#include <time.h>

///////////////////////////////////////////////////////////////////////
// get_formatted_time                                                //
// ================================================================= //
// Input: char* buffer -> Buffer to store the formatted time string  //
//        size_t size  -> Size of the buffer                         //
// Output: void                                                      //
// Purpose: Gets the current local time and writes it to the buffer  //
//          in the format "YYYY/MM/DD, HH:MM:SS".                    //
//          The result is stored in the provided buffer.             //
///////////////////////////////////////////////////////////////////////
void get_formatted_time(char* buffer, size_t size) {
    time_t now = time(NULL);
    struct tm* t = localtime(&now);
    strftime(buffer, size, "%Y/%m/%d, %H:%M:%S", t);
}