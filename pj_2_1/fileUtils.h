#ifndef FILE_UTILS_H
#define FILE_UTILS_H

void createFile(const char *dirPath, const char *fileName);
void init_log(FILE **log_fp, const char* fullPath);
void close_log(FILE **log_fp);
void write_log_contents(FILE **log_fp, const char *contents);

#endif