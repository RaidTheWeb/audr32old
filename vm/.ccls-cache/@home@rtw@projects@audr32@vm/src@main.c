#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <unistd.h>
#include <stdint.h>
#include <sys/stat.h>

#include "ram.h"
#include "vm.h"

uint32_t optip = ADDR_ROM;
size_t optramsize = RAMMINIMUM; // ensure we start of using the minimum required RAM to actually operate as default
char *optbootrom = "boot.rom";
char *optramimage = NULL;

static void usage(char *prog) {
    fprintf(stderr, "Usage: %s [-ramsize ramsize] [-rom bootrom] [-ip initial] [[-disk image] ...]\n", prog);
    fprintf(stderr, "       -ramsize ramsize, select ram size in MB\n");
    fprintf(stderr, "       -rom bootrom, select boot ROM\n");
    fprintf(stderr, "       -disk image, attach disk image\n");
    fprintf(stderr, "       -ip initial, set initial IP register\n");
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
                if(!strcmp(argv[i], "-disk")) { drives[driveptr++] = strdup(argv[++i]); break; }
                usage(argv[0]);
            case 'i':
                if(!strcmp(argv[i], "-ip")) { optip = strtol(argv[++i], NULL, 16); break; }
                usage(argv[0]);
            case 'r': 
                if(!strcmp(argv[i], "-ramimage")) { optramimage = strdup(argv[++i]); break; }
                if(!strcmp(argv[i], "-ramsize")) { optramsize = strtol(argv[++i], NULL, 10) * MB; break; }
                if(!strcmp(argv[i], "-rom")) { optbootrom = argv[++i]; break; }
                usage(argv[0]);
            default: usage(argv[0]);
        }
    }

    vm.drives = drives;
    vm.drivenum = driveptr;
    run(optramsize); 

    return 0;
}
