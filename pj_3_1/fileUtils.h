#ifndef FILE_UTILS_H
#define FILE_UTILS_H

void createFile(const char *dirPath, const char *fileName);
void init_log(FILE **log_fp, const char* fullPath);
void close_log(FILE *log_fp);
void write_log_contents(FILE *log_fp, const char *contents);
int read_file_to_buffer(FILE *fp, char *buffer, size_t bufsize);
void write_cache_file(FILE *cache_fp, const char *response, size_t size);

#endif
