#ifndef __INTERRUPTS_H__
#define __INTERRUPTS_H__

#include <stdint.h>

#define MAX_INTERRUPTS 64

typedef struct {
    uint16_t busid; // source
    uint16_t num; // interrupt number
} interrupt_t;

#endif