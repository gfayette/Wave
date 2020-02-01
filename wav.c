#include "wav.h"

int find_substring_position(char* contents, int search_length, char* search_string, int search_string_length) {
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

void print_stats(wav_file* wav, char* filename) {

    printf("*************************************************\n\n");
    printf("chunk id  %.4s\n", wav->chunk_id);
    printf("chunk size %u\n", wav->chunk_size);
    printf("file format %.4s\n\n", wav->file_format);

    //printf("format id %.4s\n", wav->format_id);
    printf("format size %i\n", wav->format_size);
    printf("format type %i\n", wav->format_type);
    printf("num channels %i\n", wav->num_channels);
    printf("sample rate %i\n", wav->sample_rate);
    printf("byte rate %i\n", wav->byte_rate);
    printf("block alignment %i\n", wav->block_alignment);
    printf("bits per sample %i\n\n", wav->bits_per_sample);

    //printf("data id %.4s\n", wav->data_id);
    printf("data size %i\n", wav->data_size);
    //printf("data position %i\n", wav->data_position);
    //printf("data pointer %p\n", wav->data_pointer);
    //printf("audio data position %i\n\n", wav->audio_data_position);

    printf("sample size in bytes %i\n", wav->sample_size_in_bytes);
    printf("num samples %i\n\n", wav->num_samples);
    printf("*************************************************\n\n");
}

wav_file* parse(char* contents) {

    wav_file* parsed_file = malloc(sizeof(wav_file));
    memcpy(parsed_file->chunk_id, contents, 12);

    int fmt_position = find_substring_position(contents, parsed_file->chunk_size + 8, "fmt ", 4);
    if (fmt_position == -1) {
        printf("Error - No \"fmt \" section");
        return NULL;
    }
    memcpy(parsed_file->format_id, contents + fmt_position, 24);

    int data_position = find_substring_position(contents, parsed_file->chunk_size + 8, "data", 4);
    if (data_position == -1) {
        printf("Error - No \"data\" section");
        return NULL;
    }

    memcpy(parsed_file->data_id, contents + data_position, 8);

    parsed_file->data_pointer = contents + data_position + 8;
    parsed_file->file_size = parsed_file->chunk_size + 8;
    parsed_file->format_position = fmt_position;
    parsed_file->data_position = data_position;
    parsed_file->audio_data_position = data_position + 8;
    parsed_file->data_end_position = parsed_file->data_position + parsed_file->data_size;
    parsed_file->bytes_after_data = parsed_file->file_size - parsed_file->data_end_position;
    parsed_file->sample_size_in_bytes = parsed_file->num_channels * parsed_file->bits_per_sample / 8;
    parsed_file->num_samples = parsed_file->data_size / parsed_file->sample_size_in_bytes;

    return parsed_file;
}

void reverse_data(char* source, int num_samples, int sample_size) {
    char* temp_sample = malloc(sample_size * sizeof(char));
    for (int i = 0; i < num_samples / 2; ++i) {
        memcpy(temp_sample, source + i * sample_size, sample_size);
        memcpy(source + i * sample_size, source + (num_samples - i - 1) * sample_size, sample_size);
        memcpy(source + (num_samples - i - 1) * sample_size, temp_sample, sample_size);
    }
    free(temp_sample);
}

void stretch_data(char* source, char* destination, int num_samples, int sample_size, double time_multiplier) {
    for (int i = 0; i < num_samples; ++i) {
        memcpy(destination + i * sample_size, source + (int)(i / time_multiplier) * sample_size, sample_size);
    }
}