#ifndef __BUS_H__
#define __BUS_H__

#include <stddef.h>

#include "vm.h"

enum {
    BUS_BYTE,
    BUS_WORD,
    BUS_DWORD,
    BUS_ERR
};

void write_byte(uint8_t *buffer, uint32_t addr, uint8_t byte);
void write_word(uint8_t *buffer, uint32_t addr, uint16_t word);
void write_dword(uint8_t *buffer, uint32_t addr, uint32_t dword);
uint8_t read_byte(uint8_t *buffer, uint32_t addr);
uint16_t read_word(uint8_t *buffer, uint32_t addr);
uint32_t read_dword(uint8_t *buffer, uint32_t addr);

int load_rom(char *rompath);

int write_bus(uint32_t addr, uint32_t type, uint32_t value);
int read_bus(uint32_t addr, uint32_t type, uint32_t *value);
void init_bus(uint32_t ramsize);

#endif
