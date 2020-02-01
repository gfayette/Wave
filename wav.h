#ifndef H_WAV
#define H_WAV

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

typedef struct wav_file {
    // RIFF header chunk
    char chunk_id[4];
    int chunk_size;
    char file_format[4];

    // fmt chunk
    char format_id[4];
    int format_size;
    short format_type;
    short num_channels;
    int sample_rate;
    int byte_rate;
    short block_alignment;
    short bits_per_sample;

    // data chunk
    char data_id[4];
    int data_size;

    // data pointer
    char *data_pointer;

    // useful file metrics
    int file_size;
    int format_position;
    int data_position;
    int audio_data_position;
    int data_end_position;
    int bytes_after_data;
    int sample_size_in_bytes;
    int num_samples;
} wav_file;

int find_substring_position(char *contents, int search_length, char *search_string, int search_string_length);

void print_stats(wav_file* wav, char* filename);

wav_file *parse(char *contents);

void reverse_data(char *source, int num_samples, int sample_size);

void stretch_data(char *source, char *destination, int num_samples, int sample_size, double time_multiplier);

#endif
