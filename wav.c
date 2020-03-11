#include "wav.h"

// A function for locating the position of a sequence of chars within within a char array.
// Returns -1 if the subsequence cannot be found.
int find_subsequence_position(char* contents, int search_length, char* search_string, int search_string_length) {
    for (int i = 0; i < search_length - search_string_length; ++i) {
        int found_string = 1;
        for (int j = 0; j < search_string_length; ++j) {
            if (*(contents + i + j) != *(search_string + j)) {
                found_string = 0;
                j = search_string_length;
            }
        }
        if (found_string != 0) {
            return i;
        }
    }
    return -1;
}

// A function for reading a wav file and detecting errors.
char* read_wav_file(char* file_name) {

    char* file_in;
    int bytes_read = read_file(file_name, &file_in);

    // Return NULL If there was an error reading the file
    if (bytes_read == -1) {
        return NULL;
    }

    // Return NULL If the file is not at least the minimum wav file size
    if (bytes_read < 44) {
        printf("Error - Not a WAVE file.\n");
        return NULL;
    }

    // Return NULL if the file is not a valid wav file
    if (find_subsequence_position(file_in, 4, "RIFF", 4) != 0 ||
            find_subsequence_position(file_in, 12, "WAVE", 4) != 8) {
        printf("Error - Not a WAVE file.\n");
        return NULL;
    }

    // Return NULL if the RIFF chunk size is incorrect
    if (bytes_read != *(int*)(file_in + 4) + 8) {
        printf("File Corrupted. Incorrect chunk size.\n");
        return NULL;
    }

    return file_in;
}

// A function for printing information about a wav file to the user.
void print_stats(wav_file* wav, char* filename) {

    printf("*************************************************\n\n");
    printf("File name:          %s\n", filename);
    printf("File size:          %u bytes\n", wav->file_size);
    printf("Channels:           %i\n", wav->num_channels);
    printf("Sample rate:        %i Hz\n", wav->sample_rate);
    printf("Bits per sample:    %i\n", wav->bits_per_sample);
    printf("Audio data size:    %i bytes\n", wav->data_size);
    printf("\n*************************************************\n\n");
}

// A function for parsing a wav file and storing information about the file
// in a wav_file struct.
wav_file* parse(char* contents) {

    // Allocate memory for the wav_file
    wav_file* parsed_file = malloc(sizeof(wav_file));

    // If malloc fails to allocate memory, print an error message and return NULL
    if (parsed_file == NULL) {
        printf("Error allocating memory for wav_file.\n\n");
        return NULL;
    }

    // Copy the RIFF header into the wav_file
    memcpy(parsed_file->chunk_id, contents, 12);

    // Locate the "fmt " chunk. If there is no "fmt " chunk in the file, print
    // an error message and return NULL
    int fmt_position = find_subsequence_position(contents, parsed_file->chunk_size + 8, "fmt ", 4);
    if (fmt_position == -1) {
        printf("Error - No \"fmt \" section.");
        return NULL;
    }

    // Copy the "fmt " chunk into the wav_file
    memcpy(parsed_file->format_id, contents + fmt_position, 24);

    // Locate the "data" chunk. If there is no "data" chunk in the file, print
    // an error message and return NULL
    int data_position = find_subsequence_position(contents, parsed_file->chunk_size + 8, "data", 4);
    if (data_position == -1) {
        printf("Error - No \"data\" section.");
        return NULL;
    }

    // Copy the "data" chunk into the wav_file
    memcpy(parsed_file->data_id, contents + data_position, 8);

    // Assign data pointer and file metrics
    parsed_file->data_pointer = contents + data_position + 8;
    parsed_file->file_size = parsed_file->chunk_size + 8;
    parsed_file->format_position = fmt_position;
    parsed_file->data_position = data_position;
    parsed_file->audio_data_position = data_position + 8;
    parsed_file->data_end_position = parsed_file->data_position + parsed_file->data_size;
    parsed_file->bytes_after_data = parsed_file->file_size - parsed_file->data_end_position;
    parsed_file->all_channel_sample_size_in_bytes = parsed_file->num_channels * parsed_file->bits_per_sample / 8;
    parsed_file->num_all_channel_samples = parsed_file->data_size / parsed_file->all_channel_sample_size_in_bytes;

    return parsed_file;
}

// A function for removing the metadata from a wav file.
void remove_metadata(char** file_in, wav_file** wav_pointer) {

    wav_file* wav_in = *wav_pointer;

    // Calculate the new chunk size without metadata.
    // Return if there is no metadata in the file.
    int new_chunk_size = 36 + wav_in->data_size;
    if (new_chunk_size == wav_in->chunk_size) {
        printf("There is no metadata in this file.\n\n");
        return;
    } else {
        printf("Removing %i bytes of metadata.\n\n", wav_in->chunk_size - new_chunk_size);
    }

    // Allocate memory for the new file, return on error.
    char* file_out = malloc((new_chunk_size + 8) * sizeof(char));
    if (file_out == NULL) {
        printf("Error allocating memory for file.\n\n");
        return;
    }

    memcpy(file_out, *file_in, 12); // Copy the RIFF header to the new file
    *(int*)(file_out + 4) = new_chunk_size; // Update the file's chunk size
    memcpy(file_out + 12, *file_in + wav_in->format_position, 24); // Copy the "fmt" chunk
    memcpy(file_out + 36, *file_in + wav_in->data_position, wav_in->data_size + 8); // Copy the "data" chunk

    // Parse the new file, free new file and return on error.
    wav_file* wav_out = parse(file_out);
    if (wav_out == NULL) {
        free(file_out);
        return;
    }

    // Free the old file and reassign file and wav_file pointers.
    free(*file_in);
    *file_in = file_out;
    free(*wav_pointer);
    *wav_pointer = wav_out;
}

// A function for embedding a hidden file within a wav file.
void push_back_file(char** file_in, wav_file** wav_pointer, char* embedded_filename) {

    char* embedded_file;
    wav_file* wav_in = *wav_pointer;

    // Read the embedded file, return on error.
    int embedded_file_size = read_file(embedded_filename, &embedded_file);
    if (embedded_file_size == -1) {
        return;
    }

    // Calculate the embedded file's chunk size and allocate memory for the new file, return on error.
    int new_chunk_size = embedded_file_size;
    char* file_out = malloc((wav_in->file_size + new_chunk_size + 8) * sizeof(char));
    if (file_out == NULL) {
        printf("Error allocating memory for file.\n\n");
        return;
    }

    // Copy the original file into the new file and update the new file's chunk size.
    memcpy(file_out, *file_in, wav_in->file_size);
    *(int*)(file_out + 4) = wav_in->chunk_size + new_chunk_size + 8;

    // Create embedded file chunk and copy the embedded file into the new file.
    memcpy(file_out + wav_in->file_size, "file", 4);
    *(int*)(file_out + wav_in->file_size + 4) = new_chunk_size;
    memcpy(file_out + wav_in->file_size + 8, embedded_file, new_chunk_size);

    // Parse the new file, free new file and return on error.
    wav_file* wav_out = parse(file_out);
    if (wav_out == NULL) {
        free(file_out);
        return;
    }

    // Free the old file and reassign file and wav_file pointers.
    free(embedded_file);
    free(*file_in);
    *file_in = file_out;
    free(*wav_pointer);
    *wav_pointer = wav_out;
}

// A function for removing the oldest hidden file within a wav file.
void pop_front_file(char** file_in, wav_file** wav_pointer, char* extracted_file_name) {

    wav_file* wav_in = *wav_pointer;

    // Locate the first hidden file, return if none exist.
    int file_chunk_position = find_subsequence_position(*file_in + wav_in->data_end_position, wav_in->bytes_after_data - 4, "file", 4);
    if (file_chunk_position == -1) {
        printf("There are no embedded files.\n\n");
        return;
    }

    // Assign variables for extracting the hidden file.
    file_chunk_position += wav_in->data_end_position;
    int file_chunk_size = *(int*)(*file_in + file_chunk_position + 4);

    // Allocate memory for the new file, return on error.
    char* file_out = malloc((wav_in->file_size - (file_chunk_size + 8)) * sizeof(char));
    if (file_out == NULL) {
        printf("Error allocating memory for file.\n\n");
        return;
    }

    // Copy to the beginning of the new file and update the new file's chunk size
    memcpy(file_out, *file_in, file_chunk_position);
    *(int*)(file_out + 4) = wav_in->chunk_size - (file_chunk_size + 8);

    // Copy to the end of the new file.
    int end_of_file_chunk = file_chunk_position + file_chunk_size + 8;
    memcpy(file_out + file_chunk_position, *file_in + end_of_file_chunk, wav_in->file_size - end_of_file_chunk);

    // Write the extracted file to disk.
    write_file(extracted_file_name, *file_in + file_chunk_position + 8, file_chunk_size);

    // Parse the new file, free new file and return on error.
    wav_file* wav_out = parse(file_out);
    if (wav_out == NULL) {
        free(file_out);
        return;
    }

    // Free the old file and reassign file and wav_file pointers.
    free(*file_in);
    *file_in = file_out;
    free(wav_in);
    *wav_pointer = wav_out;
}

// A function for stretching and copying audio data from one wav file to another.
void stretch_data(char* source, char* destination, int num_samples, int sample_size, double time_multiplier) {
    for (int i = 0; i < num_samples; ++i) {
        memcpy(destination + i * sample_size, source + (int)(i / time_multiplier) * sample_size, sample_size);
    }
}

// A function for stretching the audio data chunk in a wav file by a given factor.
char* stretch_data_chunk(char* file_in, wav_file* wav_in, double time_multiplier) {

    // Calculate the new new file's audio data size and chunk size.
    int new_data_size = (int)(wav_in->data_size * time_multiplier);
    new_data_size -= new_data_size % wav_in->all_channel_sample_size_in_bytes;
    int new_chunk_size = wav_in->chunk_size - wav_in->data_size + new_data_size;

    // Allocate memory for the new file, return NULL on error.
    char* file_out = malloc((new_chunk_size + 8) * sizeof(char));
    if (file_out == NULL) {
        printf("Error allocating memory for file.\n\n");
        return NULL;
    }

    // Copy to the beginning of the new file and reassign the file's chunk size and data size.
    memcpy(file_out, file_in, wav_in->audio_data_position);
    *(int*)(file_out + 4) = new_chunk_size;
    *(int*)(file_out + wav_in->data_position + 4) = new_data_size;

    // Copy to the end of the new file.
    memcpy(file_out + wav_in->audio_data_position + new_data_size, file_in + wav_in->audio_data_position + wav_in->data_size,
           new_chunk_size + 8 - (wav_in->audio_data_position + new_data_size));

    return file_out;
}

// A function for time-stretching the audio in a wav file by a given factor.
void stretch_audio(char** file_in, wav_file** wav_pointer, double time_multiplier) {

    wav_file* wav_in = *wav_pointer;

    // Create a new file with a stretched data chunk, return on error.
    char* file_out = stretch_data_chunk(*file_in, wav_in, fabs(time_multiplier));
    if (file_out == NULL) {
        return;
    }

    // Parse the new file, free new file and return on error.
    wav_file* wav_out = parse(file_out);
    if (wav_out == NULL) {
        free(file_out);
        return;
    }

    // Stretch and copy audio data to the new file.
    stretch_data(wav_in->data_pointer, wav_out->data_pointer, wav_out->num_all_channel_samples, wav_out->all_channel_sample_size_in_bytes,
                 fabs(time_multiplier));

    // Reverse the audio data if stretching by a negative factor.
    if (time_multiplier < 0) {
        reverse_audio(file_out + wav_out->audio_data_position, wav_out->num_all_channel_samples, wav_out->all_channel_sample_size_in_bytes);
    }

    // Free the old file and reassign file and wav_file pointers.
    free(*file_in);
    *file_in = file_out;
    free(*wav_pointer);
    *wav_pointer = wav_out;
}

// A function for reversing the audio data in a wav file.
void reverse_audio(char* source, int num_samples, int sample_size) {
    char* temp_sample = malloc(sample_size * sizeof(char));
    for (int i = 0; i < num_samples / 2; ++i) {
        memcpy(temp_sample, source + i * sample_size, sample_size);
        memcpy(source + i * sample_size, source + (num_samples - i - 1) * sample_size, sample_size);
        memcpy(source + (num_samples - i - 1) * sample_size, temp_sample, sample_size);
    }
    free(temp_sample);
}