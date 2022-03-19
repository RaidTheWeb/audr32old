#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "common.h"
#include "interrupts.h"
#include "io.h"
#include "vm.h"

int interruptpending = 0;
interrupt_t interruptbuffer[MAX_INTERRUPTS + 1];
int interruptpointer = 0;

struct IoTable iotable;
struct InterruptDescriptorTable idtable;

int currentmode = 0;

static void removeBuffer(int index) {
    if(index < 0 || index >= MAX_INTERRUPTS) return;
    interrupt_t another[MAX_INTERRUPTS + 1];
    for(size_t i = 0, k = 0; i < MAX_INTERRUPTS + 1; i++) {
        if(i == index) continue;
        another[k++] = interruptbuffer[i];
    }
    
    for(size_t i = 0; i < MAX_INTERRUPTS + 1; i++) {
        interruptbuffer[i] = another[i];
    }
    interruptpointer--;
}

void handleint(interrupt_t);

void interrupt_trigger(uint16_t src, uint16_t num) {
    interruptpending = 1;
    interrupt_t inter = { .busid = src, .num = num };
    handleint(inter);
    interruptbuffer[interruptpointer++] = inter;
}

static interrupt_t nullinterrupt = { .busid = 0, .num = 0 }; // NULL interrupt

interrupt_t interrupt_read() {
    if(interruptpointer < 0) return nullinterrupt;
    if(interruptpending == 0) return nullinterrupt;
    interrupt_t inter = interruptbuffer[interruptpointer--];
    if(interruptpointer < 0) interruptpending = 0;
    return inter;
}

void handleint(interrupt_t interrupt) {
    if(iotable.ioentries[interrupt.num].set) {
        iotable.ioentries[interrupt.num].handle();
    }
    if(idtable.intent[interrupt.num].set) {
        *vm.stacktop = vm.regs[REG_IP];
        vm.stacktop++;
        vm.regs[REG_SP]++;
        vm.regs[REG_IP] = idtable.intent[interrupt.num].addr;
    }
}

void wait_until_triggered(uint16_t busid, uint16_t num) {
    while(1) {
        for(size_t i = 0; i < MAX_INTERRUPTS; i++) {
            if(interruptbuffer[i].num == num) { //&& interruptbuffer[i].busid == busid) {
                printf("awaited interrupt 0x%04x triggered.\n", num);
                removeBuffer(i);
                return;
            }
        }
        FOR_DEVICES(id, dev) {
            if(dev->id == 0 || !dev->tick) continue;
            dev->tick(dev);
        }
        vm.tsc++;
    }
}

static uint32_t interrupt_poll(device_t *dev) {
    return interruptpending;
}

static void interrupt_pull(device_t *dev, uint32_t data) {
    if(currentmode == 0) {
        uint16_t mode = (data & 0xFFFF0000) >> 16;
        uint16_t number = data & 0x0000FFFF;
        printf("0x%04x 0x%04x\n", mode, number);
        switch(mode) {
            case 0xEEEE: { // Await I/O interrupt number
                wait_until_triggered(0x0000, number);
                break;
            }
            case 0xFFFF: { // Set table
                currentmode = 0xFFFF;
                break;
            }
        }
    } else {
        switch(currentmode) {
            case 0xFFFF: { // Set table
                printf("Attempting to set table located at 0x%08x\n", data);
                // Implement interrupt table logic here >//<
                uint8_t entries = *(uint8_t *)&vm.memory[data++];
                printf("interrupt controller: entries=%u\n", entries);
                for(size_t i = 0; i < entries; i += 5) {
                    uint8_t num = *(uint8_t *)&vm.memory[data + i];
                    uint32_t addr = ensurebig32(*(uint32_t *)&vm.memory[data + i + 1]);
                    printf("interrupt number: 0x%02x 0x%08x\n", num, addr);
                    idtent_t idtentry = {
                        .set = 1,
                        .addr = addr
                    };
                    idtable.intent[num] = idtentry;
                }
                currentmode = 0;
                break;
            }
            default:
                currentmode = 0;
                break;
        }
    }
}

static void test_handleint(void) {
    printf("test interrupt handler called!\n");
}


#define INT_DAT 0x00
#define INT_REG 0x01
#define INT_PTR 0x02

void doint(opcodepre_t prefix) {
    switch(prefix.mode) {
        case INT_DAT: {
            uint32_t num = READ_BYTE32();
            interrupt_trigger(0x0002, num); // SRC: Interrupt Controller
            break;
        }
        case INT_REG: {
            uint32_t num = GET_REGISTER32(READ_BYTE());
            interrupt_trigger(0x0002, num); // SRC: Interrupt Controller
            break;
        }
        case INT_PTR: {
            uint32_t num = GET_PTR(READ_PTR());
            interrupt_trigger(0x0002, num); // SRC: Interrupt Controller
            break;
        }
        default:
            printf("Instruction attempted to use a mode that doesn't exist! (code: 0x%02x)\n", prefix.mode);
            exit(1);
            return;
    }
}



void interrupt_init() {
    struct Device devcopy = {
        .id = io_request_id(),
        .poll = interrupt_poll,
        .pull = interrupt_pull,
        .tick = NULL,
        .destroy = NULL
    };
    strncpy(devcopy.name, "intcontr", sizeof(devcopy.name));

    /*iotableent_t entry = {
        .set = 1,
        .handle = test_handleint
    };
    iotable.ioentries[0x01] = entry;*/
    vm.devices[0x0002] = devcopy;
}
