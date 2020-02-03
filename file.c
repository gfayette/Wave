#include "file.h"

size_t read_file(char* filename, char** buffer) {

    struct stat st;

    if (stat(filename, &st)) {
        printf("The file %s does not exist.\n\n", filename);
        return -1;
    }

    size_t size = st.st_size;
    *buffer = malloc(size * sizeof(char));
    FILE* stream = fopen(filename, "r");

    if (stream == NULL) {
        printf("The file %s cannot be read.\n\n", filename);
        return -1;
    }

    size_t chars_read = fread(*buffer, sizeof(char), size, stream);

    if (size != chars_read) {
        printf("Warning, file size is %zu, but %zu bytes were read.\n\n", size, chars_read);
    }

    fclose(stream);
    return chars_read;
}

size_t write_file(char* filename, char* buffer, size_t size) {

    FILE* stream = fopen(filename, "w");

    if (stream == NULL) {
        printf("Error writing file. Unable to open file stream.\n\n");
        return -1;
    }

    size_t chars_written = fwrite(buffer, sizeof(char), size, stream);

    if (size != chars_written) {
        printf("Warning, file size is %zu, but %zu bytes were written.\n\n", size, chars_written);
    }

    fclose(stream);
    return chars_written;
}