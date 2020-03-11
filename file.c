#include "file.h"

// Function for reading a file.
// Returns the number of chars read from the file or -1 if there is an error.
size_t read_file(char* filename, char** buffer) {

    // Retrieve file information, return -1 on error
    struct stat st;
    if (stat(filename, &st)) {
        printf("The file %s does not exist.\n\n", filename);
        return -1;
    }

    // Allocate memory for the new file, return -1 on error.
    size_t size = st.st_size;
    *buffer = malloc(size * sizeof(char));
    if (*buffer == NULL) {
        printf("Error allocating memory for file %s.\n\n", filename);
        return -1;
    }

    // Open the file stream, return -1 on error
    FILE* stream = fopen(filename, "r");
    if (stream == NULL) {
        printf("The file %s cannot be read.\n\n", filename);
        return -1;
    }

    // Read the file, print an error message if unsuccessful.
    size_t chars_read = fread(*buffer, sizeof(char), size, stream);
    if (size != chars_read) {
        printf("Warning, file size is %zu, but %zu bytes were read.\n\n", size, chars_read);
    }

    fclose(stream);
    return chars_read;
}

// Function for writing to a file.
// Returns the number of chars written to the file or -1 if there is an error.
size_t write_file(char* filename, char* buffer, size_t size) {

    // Open the file stream, return -1 on error
    FILE* stream = fopen(filename, "w");
    if (stream == NULL) {
        printf("Error writing file. Unable to open file stream.\n\n");
        return -1;
    }

    // Write to the file, print an error message if unsuccessful.
    size_t chars_written = fwrite(buffer, sizeof(char), size, stream);
    if (size != chars_written) {
        printf("Warning, file size is %zu, but %zu bytes were written.\n\n", size, chars_written);
    }

    fclose(stream);
    return chars_written;
}