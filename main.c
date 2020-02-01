#include "wav.h"
#include "file.h"

char* read_wav_file(char* file_name) {
    char* file_in;
    int bytes_read = read_file(file_name, &file_in);

    if (bytes_read == -1) {
        return NULL;
    }

    if (bytes_read < 44 || find_substring_position(file_in, 5, "RIFF", 4) != 0 || find_substring_position(file_in, 13, "WAVE", 4) != 8) {
        printf("Error - Not a WAVE file\n");
        return NULL;
    }

    if (bytes_read != *(int*)(file_in + 4) + 8) {
        printf("File Corrupted. Incorrect chunk size\n");
        return NULL;
    }

    return file_in;
}

void push_back_file(char** file_in, wav_file** wav_pointer, char* embedded_filename) {

    char* embedded_file;
    wav_file* wav_in = *wav_pointer;

    int embedded_file_size = read_file(embedded_filename, &embedded_file);
    if (embedded_file_size == -1) {
        return;
    }

    int new_chunk_size = embedded_file_size;
    char* file_out = malloc((wav_in->file_size + new_chunk_size + 8) * sizeof(char));

    memcpy(file_out, *file_in, wav_in->file_size);
    *(int*)(file_out + 4) = wav_in->chunk_size + new_chunk_size + 8;

    memcpy(file_out + wav_in->file_size, "file", 4);
    *(int*)(file_out + wav_in->file_size + 4) = new_chunk_size;
    memcpy(file_out + wav_in->file_size + 8, embedded_file, new_chunk_size);

    wav_file* wav_out = parse(file_out);

    free(embedded_file);
    free(*file_in);
    *file_in = file_out;
    free(*wav_pointer);
    *wav_pointer = wav_out;
}

void pop_font_file(char** file_in, wav_file** wav_pointer, char* extracted_file_name) {

    wav_file* wav_in = *wav_pointer;

    int file_chunk_position = find_substring_position(*file_in + wav_in->data_end_position, wav_in->bytes_after_data - 4, "file", 4);

    if (file_chunk_position == -1) {
        printf("There are no embedded files in this wav\n\n");
        return;
    }

    file_chunk_position += wav_in->data_end_position;
    int file_chunk_size = *(int*)(*file_in + file_chunk_position + 4);

    char* file_out = malloc((wav_in->file_size - (file_chunk_size + 8)) * sizeof(char));
    memcpy(file_out, *file_in, file_chunk_position);
    *(int*)(file_out + 4) = wav_in->chunk_size - (file_chunk_size + 8);
    int end_of_file_chunk = file_chunk_position + file_chunk_size + 8;
    memcpy(file_out + file_chunk_position, *file_in + end_of_file_chunk, wav_in->file_size - end_of_file_chunk);

    write_file(extracted_file_name, *file_in + file_chunk_position + 8, file_chunk_size);

    wav_file* wav_out = parse(file_out);

    free(*file_in);
    *file_in = file_out;
    free(wav_in);
    *wav_pointer = wav_out;
}

char* stretch_data_chunk(char* file_in, wav_file* wav_in, double time_multiplier) {

    int new_data_size = (int)(wav_in->data_size * time_multiplier);
    new_data_size -= new_data_size % wav_in->sample_size_in_bytes;
    int new_chunk_size = wav_in->chunk_size - wav_in->data_size + new_data_size;

    char* file_out = malloc((new_chunk_size + 8) * sizeof(char));

    memcpy(file_out, file_in, wav_in->audio_data_position);
    *(int*)(file_out + 4) = new_chunk_size;
    *(int*)(file_out + wav_in->data_position + 4) = new_data_size;

    memcpy(file_out + wav_in->audio_data_position + new_data_size, file_in + wav_in->audio_data_position + wav_in->data_size,
           new_chunk_size + 8 - (wav_in->audio_data_position + new_data_size));

    return file_out;
}

void stretch_audio(char** file_in, wav_file** wav_pointer, double time_multiplier) {

    wav_file* wav_in = *wav_pointer;

    char* file_out = stretch_data_chunk(*file_in, wav_in, fabs(time_multiplier));
    wav_file* wav_out = parse(file_out);

    stretch_data(wav_in->data_pointer, wav_out->data_pointer, wav_out->num_samples, wav_out->sample_size_in_bytes,
                 fabs(time_multiplier));

    if (time_multiplier < 0) {
        reverse_data(file_out + wav_out->audio_data_position, wav_out->num_samples, wav_out->sample_size_in_bytes);
    }

    free(*file_in);
    *file_in = file_out;
    free(*wav_pointer);
    *wav_pointer = wav_out;

}

void reverse_audio(char* file, wav_file* wav_in) {
    reverse_data(file + wav_in->audio_data_position, wav_in->num_samples, wav_in->sample_size_in_bytes);
}

void remove_metadata(char** file_in, wav_file** wav_pointer) {

    wav_file* wav_in = *wav_pointer;

    int new_chunk_size = 36 + wav_in->data_size;
    if (new_chunk_size == wav_in->chunk_size) {
        printf("There is no metadata in this file\n\n");
        return;
    } else {
        printf("Removing %i bytes of metadata.\n\n", wav_in->chunk_size - new_chunk_size);
    }

    char* file_out = malloc((new_chunk_size + 8) * sizeof(char));
    memcpy(file_out, *file_in, 12);
    *(int*)(file_out + 4) = new_chunk_size;
    memcpy(file_out + 12, *file_in + wav_in->format_position, 24);
    memcpy(file_out + 36, *file_in + wav_in->data_position, wav_in->data_size + 8);

    wav_file* wav_out = parse(file_out);

    free(*file_in);
    *file_in = file_out;
    free(*wav_pointer);
    *wav_pointer = wav_out;
}

int main(int argc, char** argv) {
    printf("OPTIONS: [-t time_multiplier]  [-e embedded_file_name]  [-r removed_file_name]\n");
    printf("         [-o output_file_name]  [-m]\n\n");
    printf("         -t        Stretch audio by a given factor.\n");
    printf("         -e        Embed a given file into the wav file.\n");
    printf("         -r        Remove the oldest embedded file from the wav file.\n");
    printf("         -o        Output the current wav file.\n");
    printf("         -m        remove all metadata from the file\n\n");

    char* file;
    wav_file* wav;
    char* source_file_name;
    char* destination_file_name;

    if (argc < 3) {
        printf("Must provide 2 file names\n");
        return 0;
    } else {
        source_file_name = *(argv + 1);
        destination_file_name = *(argv + 2);
    }

    file = read_wav_file(source_file_name);
    if (file == NULL) {
        return 0;
    }

    wav = parse(file);
    if (wav == NULL) {
        return 0;
    }

    printf("\nInput file stats:\n");
    print_stats(wav, source_file_name);

    if (argc == 3) {
        printf("No options provided. Reversing audio by default.\n\n");
        reverse_audio(file, wav);
    } else {
        int current_arg = 3;
        while (current_arg < argc) {
            if (!strncmp(*(argv + current_arg), "-t", 2)) {
                double time_multiplier = strtod(*(argv + current_arg + 1), NULL);
                if (time_multiplier == 0 || fabs(time_multiplier) == HUGE_VAL) {
                    printf("Invalid time multiplier.\n\n");
                } else if (time_multiplier == 1.0) {
                    printf("Time multiplier is 1. Doing nothing.\n\n");
                } else if (time_multiplier == -1.0) {
                    printf("Reversing audio.\n\n");
                    reverse_audio(file, wav);
                } else {
                    printf("Stretching audio by a factor of %f.\n\n", time_multiplier);
                    stretch_audio(&file, &wav, time_multiplier);
                }
                current_arg += 2;
            } else if (!strncmp(*(argv + current_arg), "-e", 2)) {
                char* embedded_file_name = *(argv + current_arg + 1);
                printf("Embedding %s into wav file.\n\n", embedded_file_name);
                push_back_file(&file, &wav, embedded_file_name);
                current_arg += 2;
            } else if (!strncmp(*(argv + current_arg), "-r", 2)) {
                char* embedded_file_name = *(argv + current_arg + 1);
                printf("Extracting %s from wav file\n\n", embedded_file_name);
                pop_font_file(&file, &wav, embedded_file_name);
                current_arg += 2;
            } else if (!strncmp(*(argv + current_arg), "-o", 2)) {
                char* out_file_name = *(argv + current_arg + 1);
                printf("Writing current wav file to %s\n\n", out_file_name);
                write_file(out_file_name, file, wav->file_size);
                current_arg += 2;
            }else if (!strncmp(*(argv + current_arg), "-m", 2)) {
                remove_metadata(&file, &wav);
                ++current_arg;
            } else {
                printf("%s is an invalid option.\n\n", *(argv + current_arg));
                ++current_arg;
            }
        }
    }

    printf("Output file stats:\n");
    print_stats(wav, destination_file_name);

    write_file(destination_file_name, file, wav->file_size);
    free(wav);
    free(file);
}