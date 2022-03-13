#include <stdio.h>
#include <unistd.h>
#include <getopt.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#include "compiler.h"

char source[2048];
char output[2048];
int outputset = 0;

int main(int argc, char **argv) {
    int option;

    while(1) {
        char c;

        c = getopt (argc, argv, "ho:");
        if (c == -1) {
            break;
        }
        switch (c) {
            case 'o': // output
                strncpy(output, optarg, 2048); // copy output filepath to output variable
                outputset = 1;
                break;

            case '?':
            case 'h':
            default:
                printf("Usage: %s [-o <output>] [input].\n", argv[0]);
                return 0;
        }
    }

    argc -= optind;
    argv += optind;

    if(argc > 0) {
        strncpy(source, argv[0], 2048);
    }

    if(outputset == 0) {
        strncpy(output, "image.out", 2048);
    }

    struct stat statbuf;
    if(stat(source, &statbuf) != 0) {
        printf("Source file '%s' does not exist, exiting.\n", source);
        return 1;
    }

    FILE *file = fopen(source, "rb");

    if(file == NULL) {
        printf("Error occurred trying to open source file '%s', exiting.\n", source);
        return 1;
    }

    fseek(file, 0, SEEK_END);
    size_t size = ftell(file);
    fseek(file, 0, SEEK_SET);

    char *buffer = (char *)malloc(size);
    if(buffer == NULL) {
        printf("Error occurred trying to allocate buffer to read '%s', exiting.\n", source);
        fclose(file);
        return 1;
    }

    size_t read = fread(buffer, sizeof(char), size, file);
    if(read < size) {
        printf("Error occurred trying to read '%s', exiting.\n", source);
        fclose(file);
        free(buffer);
        return 1;
    }

    fclose(file);

    compiler(buffer, output);

    return 0;
}