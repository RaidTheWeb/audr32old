#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#include <getopt.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#include "common.h"
#include "compiler.h"

static void usage(char *prog) {
    fprintf(stderr, "Usage: %s [-b base] [-s size] [-o outfile] file [file ...]\n", prog);
    fprintf(stderr, "       -b base, force an offset\n");
    fprintf(stderr, "       -s size, force a base size for output binary\n");
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
    FILE *out = fopen("out.asm", "w");
    if(out == NULL) {
        printf("Error occurred trying to open temporary output 'out.asm', exiting. (%s)\n", strerror(errno));
        return 1;
    }
    char c;
    while(i < argc) {
        file = fopen(argv[i], "r");

        if(file == NULL) {
            printf("Error occurred trying to open source file '%s', exiting.\n", argv[i]);
            return 1;
        }
        while((c = fgetc(file)) != EOF) {
            fputc(c, out);
        }
    
        fclose(file);
        
        i++;
    }
    fclose(out);

    FILE *f = fopen("out.asm", "r");
    
    fseek(f, 0, SEEK_END);
    size_t size = ftell(f);
    fseek(f, 0, SEEK_SET);

    char *buffer = (char *)malloc(size + 1);
    if(buffer == NULL) {
        printf("error occurred trying to allocate buffer to read '%s', exiting.\n", argv[i]);
        fclose(file);
        return 1;
    }


    size_t read = fread(buffer, sizeof(char), size, f);
    if(read < size) {
        printf("error occurred trying to read '%s', exiting.\n", argv[i]);
        fclose(file);
        free(buffer);
        return 1;
    }
    buffer[size + 1] = '\0';


    compiler(buffer, outfilename, offset, basesize);

    return 0;
}
