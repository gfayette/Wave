#include "wav.h"

int main(int argc, char** argv) {

    printf("OPTIONS: [-t time_multiplier]  [-e embedded_file_name]  [-r removed_file_name]\n");
    printf("         [-o output_file_name]  [-m]\n\n");
    printf("          -t        Stretch audio by a given factor.\n");
    printf("          -e        Embed a given file into the wav file.\n");
    printf("          -r        Remove the oldest embedded file from the wav file.\n");
    printf("          -o        Output the current wav file.\n");
    printf("          -m        remove all metadata from the file\n\n");

    char* file;
    wav_file* wav;
    char* source_file_name;
    char* destination_file_name;

    if (argc < 3) {
        printf("Must provide 2 file names\n");
        exit(0);
    } else {
        source_file_name = *(argv + 1);
        destination_file_name = *(argv + 2);
    }

    file = read_wav_file(source_file_name);
    if (file == NULL) {
        exit(0);
    }

    wav = parse(file);
    if (wav == NULL) {
        exit(0);
    }

    printf("\nInput file stats:\n");
    print_stats(wav, source_file_name);

    if (argc == 3) {
        printf("No options provided. Reversing audio by default.\n\n");
        reverse_audio(wav->data_pointer, wav->num_all_channel_samples, wav->all_channel_sample_size_in_bytes);
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
                    reverse_audio(wav->data_pointer, wav->num_all_channel_samples, wav->all_channel_sample_size_in_bytes);
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
            } else if (!strncmp(*(argv + current_arg), "-m", 2)) {
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