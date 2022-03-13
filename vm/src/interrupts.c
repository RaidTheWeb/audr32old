#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "common.h"
#include "interrupts.h"
#include "io.h"
#include "vm.h"

int interruptpending = 0;
interrupt_t interruptbuffer[MAX_INTERRUPTS + 1];
int interruptpointer = 0;


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

void interrupt_trigger(uint16_t src, uint16_t num) {
    interruptpending = 1;
    interrupt_t inter = { .busid = src, .num = num };
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
                currentmode = 0;
                break;
            }
            default:
                currentmode = 0;
                break;
        }
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
    vm.devices[0x0002] = devcopy;
}