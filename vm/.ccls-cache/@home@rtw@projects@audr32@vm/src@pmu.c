#include <stdlib.h>

#include "pmu.h"

void screen_destroy(device_t *dev);

enum {
    PMU_CPUOFF
};

static void pmu_writecmd(uint16_t port, uint32_t value) {
    switch(value) {
        case PMU_CPUOFF:
            vm.running = 0; // quit
            screen_destroy(&vm.devices[0x0003]);
            break;
    }
}

static uint32_t pmu_readcmd(uint16_t port) {
    return 0;
}

void init_pmu(void) {
    vm.ports[0x60].set = 1;
    vm.ports[0x60].write = pmu_writecmd;
    vm.ports[0x60].read = pmu_readcmd;
}
