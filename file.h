#ifndef H_FILE
#define H_FILE

#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>

// Function for reading a file. Returns the number of chars read from the file.
size_t read_file(char* filename, char** buffer);

// Function for writing to a file. Returns the number of chars written to the file.
size_t write_file(char* filename, char* buffer, size_t size);

#endif
