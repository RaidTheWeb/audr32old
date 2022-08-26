#ifndef __INTERRUPTS_H__
#define __INTERRUPTS_H__

#include <stdint.h>

#define MAX_INTERRUPTS 64

typedef struct {
    uint16_t busid; // source
    uint16_t num; // interrupt number
} interrupt_t;

typedef struct {
    uint16_t set;
    void (*handle)(void);
} iotableent_t;

struct IoTable {
    iotableent_t ioentries[MAX_INTERRUPTS + 1];
};

extern struct IoTable iotable;

typedef struct {
    uint16_t set;
    uint32_t addr; // interrupt address
} idtent_t;

struct InterruptDescriptorTable {
    idtent_t intent[MAX_INTERRUPTS + 1];
};

extern struct InterruptDescriptorTable idtable;

#endif
