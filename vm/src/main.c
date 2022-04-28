#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <unistd.h>
#include <stdint.h>
#include <sys/stat.h>

#include "vm.h"

#define MAGIC_0 0x44
#define MAGIC_1 0xFF

void disasm(uint8_t *buffer, size_t size);

char image[2048];
int disassemble = 0;

int main(int argc, char **argv) {

    int option;

    while(1) {
        char c;

        c = getopt(argc, argv, "hd");
        if(c == -1) {
            break;
        }
        switch(c) {
            case 'd': // disassemble
                disassemble = 1;
                break;
            case '?':
            case 'h':
            default:
                printf("Usage: %s [-d] [image].\n", argv[0]);
                return 0;
        }
    }

    argc -= optind;
    argv += optind;

    if(argc > 0) {
        strncpy(image, argv[0], 2048);
    }

    struct stat statbuf;
    if(stat(image, &statbuf) != 0) {
        printf("Image file '%s' does not exist, exiting.\n", image);
        return 1;
    }

    FILE *file = fopen(image, "rb");

    if(file == NULL) {
        printf("Error occurred trying to open image file '%s', exiting.\n", image);
        return 1;
    }

    fseek(file, 0, SEEK_END);
    size_t size = ftell(file);
    fseek(file, 0, SEEK_SET);

    uint8_t *buffer = (uint8_t *)malloc(size);
    if(buffer == NULL) {
        printf("Error occurred trying to allocate buffer to read '%s', exiting.\n", image);
        fclose(file);
        return 1;
    }

    size_t read = fread(buffer, sizeof(uint8_t), size, file);
    if(read < size) {
        printf("Error occurred trying to read '%s', exiting.\n", image);
        fclose(file);
        free(buffer);
        return 1;
    }

    fclose(file);

    if(!(size > 2)) {
        printf("Error, image '%s' content is possibly corrupted or incorrectly setup.\n", image);
        free(buffer);
        return 1;
    }

    /*if(!(buffer[0] == MAGIC_0 && buffer[1] == MAGIC_1)) { // Ensure buffer contains magic bytes.
        printf("Error, image '%s' content is possibly corrupted or incorrectly setup.\n", image);
        free(buffer);
        return 1;
    }*/ 
    if(!disassemble)
        run(buffer, size);
    else
        //disasm(buffer, size);

    return 0;
}
