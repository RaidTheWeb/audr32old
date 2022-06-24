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
    //printf("interrupt triggered, src=0x%04x dest=0x%04x\n", src, num);
    vm.interruptpending = 1;
    interrupt_t inter = { .busid = src, .num = num };
    handleint(inter);
    interruptbuffer[interruptpointer++] = inter;
    interruptpointer %= MAX_INTERRUPTS;
//    printf("interruptpointer: %d\n", interruptpointer);
}

static interrupt_t nullinterrupt = { .busid = 0, .num = 0 }; // NULL interrupt

interrupt_t interrupt_read() {
    if(interruptpointer < 0) return nullinterrupt;
    if(vm.interruptpending == 0) return nullinterrupt;
    interrupt_t inter = interruptbuffer[interruptpointer--];
    if(interruptpointer < 0) vm.interruptpending = 0;
    return inter;
}

void handleint(interrupt_t interrupt) {
    uint16_t found = 0;
    if(iotable.ioentries[interrupt.num].set) {
        found = 1;
        iotable.ioentries[interrupt.num].handle();
        vm.interruptpending = 0;
    }
    if(idtable.intent[interrupt.num].set) {
        found = 1;
        vm.regs[REG_SP] -= 4; 
        ptr_t pointer = {
            .addr = vm.regs[REG_SP],
            .ptrmode = 0x03
        };
        SET_PTR(pointer, vm.regs[REG_IP]); 
        vm.regs[REG_IP] = idtable.intent[interrupt.num].addr;
        vm.interruptpending = 0;
    }
    void audr32_safeexception(uint32_t exc);
    if(!found) audr32_safeexception(EXC_BADINT);
}

void wait_until_triggered(uint16_t busid, uint16_t num) {
    while(1) {
        for(size_t i = 0; i < MAX_INTERRUPTS; i++) {
            if(interruptbuffer[i].num == num) { //&& interruptbuffer[i].busid == busid) { 
                removeBuffer(i);
                return;
            }
        }
        FOR_DEVICES(id, dev) {
            if(dev->id == 0 || !dev->tick) continue;
            dev->tick(dev);
        }
    }
}

static uint32_t read_porta(uint16_t port) {
    return vm.interruptpending;
}

static void write_porta(uint16_t port, uint32_t data) {
    wait_until_triggered(0x0000, data);
}

static uint32_t read_portb(uint16_t port) {
    return 0;
}

static void write_portb(uint16_t port, uint32_t data) {
    uint8_t entries = cpu_readbyte(data++);
    for(size_t i = 0; i < entries; i += 5) {
        uint8_t num = cpu_readbyte(data + i);
        uint32_t addr = cpu_readdword(data + i + 1); 
        idtent_t idtentry = {
            .set = 1,
            .addr = addr
        };
        idtable.intent[num] = idtentry;
    }

}

static void interrupt_pull(uint16_t port, uint32_t data) {
    if(currentmode == 0) {
        uint16_t mode = (data & 0xFFFF0000) >> 16;
        uint16_t number = data & 0x0000FFFF; 
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
                // Implement interrupt table logic here >//<
                uint8_t entries = *(uint8_t *)&vm.memory[data++];
                for(size_t i = 0; i < entries; i += 5) {
                    uint8_t num = *(uint8_t *)&vm.memory[data + i];
                    ptr_t addrptr = {
                        .addr = data + i + 1,
                        .ptrmode = 0x03
                    };
                    uint32_t addr = GET_PTR(addrptr); 
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

static void interruptcont_handleint(void) {
    uint8_t mode = vm.regs[REG_R10];

    switch(mode) {
        case 0x01: {
            // Load IDT
            // dx: address
            
            uint32_t address = vm.regs[REG_DX];
            uint8_t entries = *(uint8_t *)&vm.memory[address++]; // it's only one byte so no need for any endian-independant pointer logic
            for(size_t i = 0; i < entries; i += 5) {
                uint8_t num = *(uint8_t *)&vm.memory[address + i]; // likewise
                ptr_t addrptr = {
                    .addr = address + i + 1,
                    .ptrmode = 0x03
                };
                uint8_t handleraddr = GET_PTR(addrptr); // now we actually do care about endianness, use proper pointer logic here
                idtent_t idtentry = {
                    .set = 1,
                    .addr = handleraddr
                };
                idtable.intent[num] = idtentry;
            }
            break;
        }
    }
}


#define INT_DAT 0x00
#define INT_REG 0x01
#define INT_PTR 0x02

void doint(opcodepre_t prefix) {
    //printf("int instruction!\n");
    switch(prefix.mode) {
        case INT_DAT: {
            uint32_t num = READ_BYTE32();
//            printf("int 0x%08x\n", num);
            interrupt_trigger(0x0002, num); // SRC: Interrupt Controller
           // printf("triggered interrupt via instruction dat!\n");
            break;
        }
        case INT_REG: {
            uint32_t num = GET_REGISTER32(READ_BYTE());
            interrupt_trigger(0x0002, num); // SRC: Interrupt Controller
          //  printf("triggered interrupt via instruction reg!\n");
            break;
        }
        case INT_PTR: {
            uint32_t num = GET_PTR(READ_PTR());
            interrupt_trigger(0x0002, num); // SRC: Interrupt Controller
           // printf("triggered interrupt via instruction ptr!\n");
            break;
        }
        default:
            printf("Instruction attempted to use a mode that doesn't exist! (code: 0x%02x)\n", prefix.mode);
            exit(1);
            return;
    }
}

static void interrupt_tick(device_t *dev) {
    //printf("ticking interrupt controller!\n");
}

void interrupt_init(void) {
    struct Device devcopy = {
        .id = io_request_id(),
        .set = 1,
        .tick = interrupt_tick,
        .destroy = NULL
    };
    strncpy(devcopy.name, "intcontr", sizeof(devcopy.name));

    iotableent_t entry = {
        .set = 1,
        .handle = interruptcont_handleint
    };
    iotable.ioentries[0x00014] = entry;
    vm.devices[0x0002] = devcopy;

    /** Wait For Interrupt Trigger */
    vm.ports[0x3A].set = 1;
    vm.ports[0x3A].read = read_porta;
    vm.ports[0x3A].write = write_porta;

    /** Set Interrupt Descriptor Table */
    vm.ports[0x3B].set = 1;
    vm.ports[0x3B].read = read_portb;
    vm.ports[0x3B].write = write_portb;
}
