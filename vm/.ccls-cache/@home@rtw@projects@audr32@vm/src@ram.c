#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bus.h"
#include "ram.h"

void write_ram(uint32_t addr, uint32_t type, uint32_t value) {
    switch(type) {
        case BUS_BYTE:
            write_byte(vm.memory, addr, value);
            break;
        case BUS_WORD:
            write_word(vm.memory, addr, value);
            break;
        case BUS_DWORD:
            write_dword(vm.memory, addr, value);
            break;
    }
}

uint32_t read_ram(uint32_t addr, uint32_t type) {
    switch(type) {
        case BUS_BYTE:
            return read_byte(vm.memory, addr); 
        case BUS_WORD:
            return read_word(vm.memory, addr);
        case BUS_DWORD:
            return read_dword(vm.memory, addr);
    }

    return 0;
}

int load_ram(char *rampath) {
    FILE *ramfile = fopen(rampath, "r");

    if(!ramfile) {
        fprintf(stderr, "load_ram: failed to open RAM image `%s` (%s)\n", rampath, strerror(errno));
        exit(1);
    }

    fseek(ramfile, 0, SEEK_END);
    size_t size = ftell(ramfile);
    fseek(ramfile, 0, SEEK_SET);

    uint8_t *buffer = (uint8_t *)malloc(size);
    if(!buffer) {
        fprintf(stderr, "load_ram: failed to allocate buffer for `%s` (%s)\n", rampath, strerror(errno));
        exit(1);
    }

    size_t read = fread(buffer, sizeof(uint8_t), size, ramfile);
    if(read < size) {
        fprintf(stderr, "load_ram: error occurred trying to read '%s', exiting. (%s)\n", rampath, strerror(errno));
        fclose(ramfile);
        free(buffer);
        exit(1);
    }


    fclose(ramfile);

    size_t k = ADDR_RAM - ADDR_STACKRAM;
    for(size_t i = 0; i < size; i++) {
        vm.memory[k++] = buffer[i];
    }

    return 1;
}

void init_ram(uint32_t memsize) {
    if(memsize > RAMMAXIMUM) {
        fprintf(stderr, "init_ram: given size (%d) exceeds maximum of %d\n", memsize, RAMMAXIMUM);
        exit(1);
        return;
    } else if(memsize < RAMMINIMUM) {
        fprintf(stderr, "init_ram: given size (%d) does not meet minimum of %d required to operate\n", memsize, RAMMINIMUM);
        exit(1);
        return;
    }

    vm.memory = malloc(memsize);
}
