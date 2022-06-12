#include <stdio.h>
#include <unistd.h>
#include <getopt.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#include "common.h"
#include "compiler.h"

static void usage(char *prog) {
    fprintf(stderr, "Usage: %s [-o outfile] file [file ...]\n", prog);
    fprintf(stderr, "       -o outfile, output executable\n");
    exit(1);
}

#define MAXINPUT (16 * MB)

int main(int argc, char **argv) {
    char *outfilename = "a.out";
    uint32_t offset = 0x40330000; // default offset is boot ROM
    uint32_t basesize = 0;
    char *inputcomb = (char *)malloc(sizeof(char) * MAXINPUT);
    int i;
    
    for(i = 1; i < argc; i++) {
        if(*argv[i] != '-') break;

        for(int j = 1; (*argv[i] == '-') && argv[i][j]; j++) {
            switch(argv[i][j]) {
                case 'o':
                    outfilename = argv[++i];
                    break;
                case 'b': // base offset
                    offset = strtol(argv[++i], NULL, 16);
                    break;
                case 's': // forced size
                    basesize = strtol(argv[++i], NULL, 10);
                    break;
                default:
                    usage(argv[0]);
            }
        }
    }

    if(i >= argc) usage(argv[0]);

    FILE *file;
    int inputcomblen = 0;
    while(i < argc) {
        file = fopen(argv[i], "r");

        if(file == NULL) {
            printf("Error occurred trying to open source file '%s', exiting.\n", argv[i]);
            return 1;
        }
    
        fseek(file, 0, SEEK_END);
        size_t size = ftell(file);
        fseek(file, 0, SEEK_SET);
    
        char *buffer = (char *)malloc(size + 1);
        if(buffer == NULL) {
            printf("Error occurred trying to allocate buffer to read '%s', exiting.\n", argv[i]);
            fclose(file);
            return 1;
        }
    
        size_t read = fread(buffer, sizeof(char), size, file);
        if(read < size) {
            printf("Error occurred trying to read '%s', exiting.\n", argv[i]);
            fclose(file);
            free(buffer);
            return 1;
        }
        

        fclose(file);
        buffer[size + 1] = '\0';

        snprintf(inputcomb, MAXINPUT, "%s\n\n%s", strdup(inputcomb), strdup(buffer));
        inputcomblen = inputcomblen + (size + 1) + 1;
        
        i++;
    }
    inputcomb[inputcomblen + 1] = '\0';

    FILE *f = fopen("out.asm", "w");
    fwrite(inputcomb, inputcomblen, 1, f);
    fclose(f);

    compiler(inputcomb, outfilename, offset, basesize);

    return 0;
}
