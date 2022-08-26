#ifndef __DISK_H__
#define __DISK_H__

#include <stdint.h>

#define DRIVES 8

int drive_attachimage(char *drivepath);

void write_sectorcache(uint32_t addr, uint32_t type, uint32_t value);
uint32_t read_sectorcache(uint32_t addr, uint32_t type);
void init_drive();

#endif
