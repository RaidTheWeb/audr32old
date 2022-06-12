#include <errno.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bus.h"
#include "disk.h"
#include "ram.h"
#include "vm.h"

uint32_t busregs[10]; // bus registers

void write_fb(uint32_t, uint32_t, uint32_t);
uint32_t read_fb(uint32_t, uint32_t);
void write_textmode(uint32_t, uint32_t, uint32_t);
uint32_t read_textmode(uint32_t, uint32_t);

void write_rom(uint32_t addr, uint32_t type, uint32_t value) {
    switch(type) {
        case BUS_BYTE:
            write_byte(vm.rom, addr, value);
            break;
        case BUS_WORD:
            write_word(vm.rom, addr, value);
            break;
        case BUS_DWORD:
            write_dword(vm.rom, addr, value);
            break;
    }
}

uint32_t read_rom(uint32_t addr, uint32_t type) {
    switch(type) {
        case BUS_BYTE:
            return read_byte(vm.rom, addr);
        case BUS_WORD:
            return read_word(vm.rom, addr);
        case BUS_DWORD:
            return read_dword(vm.rom, addr);
    }
    
    return 0;
}

int load_rom(char *rompath) {
    FILE *romfile = fopen(rompath, "r");

    if(!romfile) {
        fprintf(stderr, "load_rom: failed to open ROM `%s` (%s)\n", rompath, strerror(errno));
        exit(1);
    }

    fread(&vm.rom, 64 * KB, 1, romfile);

    fclose(romfile);

    return 1;
}


void write_byte(uint8_t *buffer, uint32_t addr, uint8_t byte) {
    buffer[addr] = byte;
}

void write_word(uint8_t *buffer, uint32_t addr, uint16_t word) {
    buffer[addr] = word >> 8;
    buffer[addr + 1] = word;
}

void write_dword(uint8_t *buffer, uint32_t addr, uint32_t dword) {
    buffer[addr] = dword >> 24;
    buffer[addr + 1] = dword >> 16;
    buffer[addr + 2] = dword >> 8;
    buffer[addr + 3] = dword;
}

uint8_t read_byte(uint8_t *buffer, uint32_t addr) {
    return buffer[addr];
}

uint16_t read_word(uint8_t *buffer, uint32_t addr) {
    return (read_byte(buffer, addr) << 8) | (read_byte(buffer, addr + 1));
}

uint32_t read_dword(uint8_t *buffer, uint32_t addr) {
    return (read_word(buffer, addr) << 16) + (read_word(buffer, addr + 2));
}

int write_bus(uint32_t addr, uint32_t type, uint32_t value) {
    if(addr >= ADDR_STACKRAM && addr < ADDR_RAMEND) {
        addr -= ADDR_STACKRAM;
        write_ram(addr, type, value);
    } else if(addr >= ADDR_FRAMEBUFFER && addr < ADDR_FRAMEBUFFEREND) {
        addr -= ADDR_FRAMEBUFFER;
        write_fb(addr, type, value);
    } else if(addr >= ADDR_TEXTBUFFER && addr < ADDR_TEXTBUFFEREND) {
        addr -= ADDR_TEXTBUFFER;
        write_textmode(addr, type, value);
    } else if(addr >= ADDR_ROM && addr < ADDR_ROMEND) {
        // it's read only (maybe throw an illegal memory access exception)
        return 1;
    } else if(addr >= ADDR_SECTORCACHE && addr < ADDR_SECTORCACHEEND) {
        addr -= ADDR_SECTORCACHE;
        write_sectorcache(addr, type, value);
    } else if(addr >= ADDR_BUSREGISTERS && addr < ADDR_BUSREGISTERSEND) {
        // it's read only (maybe throw an illegal memory access exception)
        return 1;
    }
    return 1;
}

int read_bus(uint32_t addr, uint32_t type, uint32_t *value) {
    if(addr >= ADDR_STACKRAM && addr < ADDR_RAMEND) {
        addr -= ADDR_STACKRAM; // ensure we're only using the direct memory address to the RAM
        *value = read_ram(addr, type);
    } else if(addr >= ADDR_FRAMEBUFFER && addr < ADDR_FRAMEBUFFEREND) {
        addr -= ADDR_FRAMEBUFFER;
        *value = read_fb(addr, type);
    } else if(addr >= ADDR_TEXTBUFFER && addr < ADDR_TEXTBUFFEREND) {
        addr -= ADDR_TEXTBUFFER;
        *value = read_textmode(addr, type);
    } else if(addr >= ADDR_ROM && addr < ADDR_ROMEND) {
        addr -= ADDR_ROM;
        *value = read_rom(addr, type);
    } else if(addr >= ADDR_SECTORCACHE && addr < ADDR_SECTORCACHEEND) {
        addr -= ADDR_SECTORCACHE;
        *value = read_sectorcache(addr, type);
    } else if(addr >= ADDR_BUSREGISTERS && addr < ADDR_BUSREGISTERSEND) {
        addr -= ADDR_BUSREGISTERS;
        printf("bus: 0x%08x\n", addr);
        if(type == BUS_DWORD) {
            *value = busregs[addr];
            return 1;
        }
        return BUS_ERR;
    }
    return 1;
}

void init_bus(uint32_t memsize) {
    init_ram(memsize); // initialise ram

}
