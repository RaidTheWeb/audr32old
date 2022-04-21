#ifndef __IO_H__
#define __IO_H__

#include <stdint.h>

#define MAX_PORTS 0xFFFF



typedef struct Device {
    uint16_t id;
    uint8_t set; // to ensure working
    char name[32];
    void *state;
    void (*tick)(struct Device *);
    uint32_t (*poll)(struct Device *);
    void (*pull)(struct Device *, uint32_t);
    void (*destroy)(struct Device *);
} device_t;

#define FOR_DEVICES(_i, _d)                 \
    unsigned long long _i;                  \
    device_t *_d;                           \
    for(_i = 0, _d = &vm.devices[_i];       \
            _i < MAX_PORTS;                 \
            _i++,_d = &vm.devices[_i])      \

uint16_t io_request_id();

#endif
