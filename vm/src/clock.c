#include <stdint.h>
#include <time.h>

#include "clock.h"
#include "vm.h"

enum {
    CLOCK_SETINTV,
    CLOCK_GETEPOCHS,
    CLOCK_GETEPOCHMS,
    CLOCK_SETEPOCHS,
    CLOCK_SETEPOCHMS,

    CLOCK_GETYEAR,
    CLOCK_GETMON,
    CLOCK_GETDAY,
    CLOCK_GETHOUR,
    CLOCK_GETMIN,
    CLOCK_GETSEC
};

time_t curtime;

time_t curtimesec = 0;
uint32_t curtimems = 0;

uint32_t intervalms = 0; // for interval based interrupts (kind of like the PIT)
uint32_t intervalcounter = 0;

uint32_t datamod;

int modified = 0;

void clock_tick(uint32_t dt) {
    if(!modified) time(&curtime);
    else {
        curtimems += dt;
        if(curtimems >= 1000) {
            curtimems -= 1000;
            curtimesec++;
        }
    }

    intervalcounter += dt;

    if(intervalcounter >= intervalms) {
        void interrupt_trigger(uint16_t from, uint16_t to);

        intervalcounter -= intervalms;
    }
}

static void clock_writemod(uint16_t port, uint32_t data) {
    datamod = data;
}

static uint32_t clock_readmod(uint16_t port) {
    return datamod;
}

static void clock_writecmd(uint16_t port, uint32_t data) {
    switch(data) {
        case CLOCK_SETINTV:
            intervalms = datamod;
            intervalcounter = 0; // zero initialise counter
            break;
        case CLOCK_GETEPOCHS:
            if(modified) datamod = curtimesec;
            else datamod = curtime;
            break;
        case CLOCK_GETEPOCHMS:
            if(modified) datamod = curtimems;
            else datamod = curtime * 1000; // make sure it's in milliseconds

            // something progressy
            break;

        // high level abstractions for year, month, day, hour, minute and second
        case CLOCK_GETYEAR: {
            struct tm *tmt;
            if(modified) tmt = localtime(&curtimesec);
            else tmt = localtime(&curtime);
            datamod = tmt->tm_year;
            break;
        }
        case CLOCK_GETMON: {
            struct tm *tmt;
            if(modified) tmt = localtime(&curtimesec);
            else tmt = localtime(&curtime);
            datamod = tmt->tm_mon;
            break;
        }
        case CLOCK_GETDAY: {
            struct tm *tmt;
            if(modified) tmt = localtime(&curtimesec);
            else tmt = localtime(&curtime);
            datamod = tmt->tm_mday;
            break;
        }
        case CLOCK_GETHOUR: {
            struct tm *tmt;
            if(modified) tmt = localtime(&curtimesec);
            else tmt = localtime(&curtime);
            datamod = tmt->tm_hour;
            break;
        }
        case CLOCK_GETMIN: {
            struct tm *tmt;
            if(modified) tmt = localtime(&curtimesec);
            else tmt = localtime(&curtime);
            datamod = tmt->tm_min;
            break;
        }
        case CLOCK_GETSEC: {
            struct tm *tmt;
            if(modified) tmt = localtime(&curtimesec);
            else tmt = localtime(&curtime);
            datamod = tmt->tm_sec;
            break;
        }
        
        case CLOCK_SETEPOCHS:
            curtimesec = datamod;
            modified = 1;
            break;
        case CLOCK_SETEPOCHMS:
            curtimems = datamod;
            modified = 1;
            break;
    } 
}

static uint32_t clock_readcmd(uint16_t port) {
    return 0;
}

void init_clock(void) {
    time(&curtime); // initialise clock
    
    vm.ports[0x40].set = 1;
    vm.ports[0x40].read = clock_readmod;
    vm.ports[0x40].write = clock_writemod;

    vm.ports[0x41].set = 1;
    vm.ports[0x41].read = clock_readcmd;
    vm.ports[0x41].write = clock_writecmd;
}
