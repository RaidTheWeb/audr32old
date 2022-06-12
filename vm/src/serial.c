#include <stdio.h>

#include "serial.h"
#include "vm.h"

#define SERIALBUFFERSIZE 128

typedef struct {
    uint32_t dataval;
    uint32_t dointerrupts;
    uint8_t lastchar;
    uint32_t buffer[SERIALBUFFERSIZE];
    size_t transindex;
    size_t sendindex;
    uint32_t readbusy;
    uint32_t writebusy;
} serialport_t;

#define SERIALPORTS 1

enum {
    SERIAL_CMDDOINT = 0x4,
    SERIAL_CMDDONTINT = 0x5
};

serialport_t serialports[SERIALPORTS];

void interrupt_trigger(uint16_t, uint16_t);

void serial_tick(uint32_t dt) {
    for(int port = 0; port < SERIALPORTS; port++) {
        serialport_t *curport = &serialports[port];

        while(dt) {
            if(curport->sendindex < curport->transindex) printf("%u\n", curport->buffer[curport->sendindex++]);
            else break;

            if(curport->sendindex == curport->transindex) {
                curport->sendindex = 0;
                curport->transindex = 0;
                curport->writebusy = 0;

                if(curport->dointerrupts) interrupt_trigger(0x10 + port, 0x04); // trigger serial interrupt
            }

            dt--;
        }

        fflush(stdout);
    }
}

static void serial_writecmd(uint16_t port, uint32_t data) {
    serialport_t *curport;

    if(port == 0x11) {
        curport = &serialports[0];
    }

    switch(data) {
        case SERIAL_CMDDOINT:
            curport->dointerrupts = 1;
            break;
        case SERIAL_CMDDONTINT:
            curport->dointerrupts = 0;
            break;
    }
}

static uint32_t serial_readcmd(uint16_t port) {
    serialport_t *curport;

    if(port == 0x11) {
        curport = &serialports[0];
    }

    return curport->writebusy;
}

static void serial_write(uint16_t port, uint32_t data) {
    serialport_t *curport;

    if(port == 0x10) {
        curport = &serialports[0];
    }

    if(curport->transindex == SERIALBUFFERSIZE) return;

    curport->buffer[curport->transindex++] = data;

    if(curport->transindex == SERIALBUFFERSIZE) curport->writebusy = 1;
}

static uint32_t serial_read(uint16_t port) {
    serialport_t *curport;

    if(port == 0x10) {
        curport = &serialports[0];
    }

    return 0xFFFFFFFF;
}

void init_serial(void) {
    // SERIAL 1
    vm.ports[0x10].set = 1;
    vm.ports[0x10].write = serial_write;
    vm.ports[0x10].read = serial_read;
    vm.ports[0x10].set = 1;
    vm.ports[0x11].write = serial_writecmd;
    vm.ports[0x11].read = serial_readcmd;

    serialports[0].lastchar = 0xFF;
}
