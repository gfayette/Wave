#ifndef H_FILE
#define H_FILE

#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>

size_t read_file(char* filename, char** buffer);

size_t write_file(char* filename, char* buffer, size_t size);

#endif
