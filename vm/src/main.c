#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <unistd.h>
#include <stdint.h>
#include <sys/stat.h>

#include "ram.h"
#include "vm.h"

size_t optramsize = RAMMINIMUM; // ensure we start of using the minimum required RAM to actually operate as default
char *optbootrom = "boot.rom";
char *optramimage = NULL;

static void usage(char *prog) {
    fprintf(stderr, "Usage: %s [-ramsize ramsize] [-rom bootrom] [[-drive drive] ...]\n", prog);
    fprintf(stderr, "       -ramsize ramsize, select ram size in MB\n");
    fprintf(stderr, "       -rom bootrom, select boot ROM\n");
    fprintf(stderr, "       -drive drive, attach drive image\n");
    exit(1);
}

int main(int argc, char **argv) {
    int i;
    char *drives[32];
    int driveptr = 0;

    for(i = 1; i < argc; i++) {
        if(*argv[i] != '-') break;
        switch(argv[i][1]) {
            case 'd':
                if(!strcmp(argv[i], "-drive")) drives[driveptr++] = strdup(argv[++i]);
                break;
            case 'r': 
                if(!strcmp(argv[i], "-ramimage")) optramimage = strdup(argv[++i]);
                if(!strcmp(argv[i], "-ramsize")) optramsize = strtol(argv[++i], NULL, 10) * MB;
                if(!strcmp(argv[i], "-rom")) optbootrom = argv[++i];
                break;
            default: usage(argv[0]);
        }
    }

    run(optramsize, drives, driveptr); 

    return 0;
}
