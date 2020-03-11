#ifndef H_WAV
#define H_WAV

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "file.h"

// A struct for holding information about a wav file.
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
    char* data_pointer;

    // useful file metrics
    int file_size;
    int format_position;
    int data_position;
    int audio_data_position;
    int data_end_position;
    int bytes_after_data;
    int all_channel_sample_size_in_bytes;
    int num_all_channel_samples;

} wav_file;

// A function for reading a wav file and detecting errors.
char* read_wav_file(char* file_name);

// A function for printing information about a wav file to the user.
void print_stats(wav_file* wav, char* filename);

// A function for parsing a wav file and storing information about the file
// in a wav_file struct.
wav_file* parse(char* contents);

// A function for removing the metadata from a wav file.
void remove_metadata(char** file_in, wav_file** wav_pointer);

// A function for embedding a hidden file within a wav file.
void push_back_file(char** file_in, wav_file** wav_pointer, char* embedded_filename);

// A function for removing the oldest hidden file within a wav file.
void pop_front_file(char** file_in, wav_file** wav_pointer, char* extracted_file_name);

// A function for time-stretching the audio in a wav file by a given factor.
void stretch_audio(char** file_in, wav_file** wav_pointer, double time_multiplier);

// A function for reversing the audio data in a wav file.
void reverse_audio(char* source, int num_samples, int sample_size);

#endif
