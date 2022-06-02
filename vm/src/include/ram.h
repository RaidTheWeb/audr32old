#ifndef __RAM_H__
#define __RAM_H__

#include <stddef.h>

#include "common.h"
#include "io.h"
#include "vm.h"

/** Minimum RAM Requirement          STACK    + ESTIMATED OPERABLE SIZE */
#define RAMMINIMUM                  ((2 * MB) + (16 * MB))
/** Maximum RAM Restriction */
#define RAMMAXIMUM                  (1 * GB)

int load_ram(char *rampath);

void write_ram(uint32_t addr, uint32_t type, uint32_t value);
uint32_t read_ram(uint32_t addr, uint32_t type);
void init_ram(uint32_t memsize);

#endif
