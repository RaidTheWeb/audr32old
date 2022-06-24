#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#include "vm.h"
#include "common.h"

uint16_t io_request_id() {
    uint16_t r;
    while(1) {
        r = (uint16_t)((rand() + 1) % MAX_PORTS);
        int has = 0;
        FOR_DEVICES(id, dev) {
            if(dev->id == r) has = 1;
        }
        if(!has) return r;
    }
}

void io_remove_device(uint16_t busid) {
    FOR_DEVICES(id, dev) {
        if(dev->id == busid) {
            if(dev->destroy) {
                dev->destroy(dev);
            }
            dev->id = 0; // nullify magic.
        }
    }
}

static uint32_t inx(uint16_t port) {
    printf("inx 0x%04x\n", port);
    if(port >= MAX_PORTS) {
        printf("Instruction attempted to access illegal I/O port, somehow. (code: 0x%04x)\n", port);
        exit(1);
        return 0;
    }

    device_t *dev;
    if(!vm.ports[port].set) {
        printf("Instruction attempted to poll I/O device that does not implement such method! (code: 0x%04x, '%s')\n", port, dev->name);
        exit(1);
        return 0;
    }
    return vm.ports[port].read(port);

    return 0;
}

static void outx(uint16_t port, uint32_t data) {
    if(port >= MAX_PORTS) {
        printf("Instruction attempted to access illegal I/O port, somehow. (code: 0x%04x)\n", port);
        exit(1);
        return;
    }

    device_t *dev;

    if(!vm.ports[port].set) {
        printf("Instruction attempted to poll I/O device that does not implement such method! (code: 0x%04x, '%s')\n", port, dev->name);
        exit(1);
    }
    vm.ports[port].write(port, data);
}

#define INX_REG 0x00
#define INX_PTR 0x01
#define INX_DAT 0x02

void doinx(opcodepre_t prefix) {
    printf("begin inx\n");
    switch(prefix.mode) {
        case INX_REG: {
            uint8_t reg = READ_BYTE();
            vm.regs[REG_DX] = inx(GET_REGISTER32(reg));
            break;
        }
        case INX_PTR: {
            uint32_t pointer = GET_PTR(READ_PTR());
            vm.regs[REG_DX] = inx(pointer);
            break;
        }
        case INX_DAT: {
            uint32_t data = READ_BYTE32();
            vm.regs[REG_DX] = inx(data);
            break;
        }
        default:
            printf("Instruction attempted to use a mode that doesn't exist! (code: 0x%02x)\n", prefix.mode);
            exit(1);
            return;
    }
    printf("end inx\n");
}



#define OUT_REGREG 0x00
#define OUT_REGPTR 0x01
#define OUT_REGDAT 0x02
#define OUT_PTRREG 0x03
#define OUT_PTRPTR 0x04
#define OUT_PTRDAT 0x05
#define OUT_DATREG 0x06
#define OUT_DATPTR 0x07
#define OUT_DATDAT 0x08

void dooutx(opcodepre_t prefix) {
    switch(prefix.mode) {
        case OUT_REGREG: {
            uint8_t reg = READ_BYTE();
            uint8_t reg2 = READ_BYTE();
            outx(GET_REGISTER32(reg), GET_REGISTER32(reg2));
            break;
        }
        case OUT_REGPTR: {
            uint8_t reg = READ_BYTE();
            uint32_t pointer = GET_PTR(READ_PTR());
            outx(GET_REGISTER32(reg), pointer);
            break;
        }
        case OUT_REGDAT: {
            uint8_t reg = READ_BYTE();
            uint32_t data = READ_BYTE32();
            outx(GET_REGISTER32(reg), data);
            break;
        }
        case OUT_PTRPTR: {
            uint32_t pointer = GET_PTR(READ_PTR());
            uint32_t pointer2 = GET_PTR(READ_PTR());
            outx(pointer, pointer2);
            break;
        }
        case OUT_PTRDAT: {
            uint32_t pointer = GET_PTR(READ_PTR());
            outx(pointer, READ_BYTE32());
            break;
        }
        case OUT_PTRREG: {
            uint32_t pointer = GET_PTR(READ_PTR());
            outx(pointer, GET_REGISTER32(READ_BYTE()));
            break;
        }
        case OUT_DATDAT: {
            uint32_t data = READ_BYTE32();
            uint32_t data2 = READ_BYTE32();
            outx(data, data2);
            break;
        }
        case OUT_DATPTR: {
            uint32_t data = READ_BYTE32();
            uint32_t pointer = GET_PTR(READ_PTR());
            outx(data, pointer);
            break;
        }
        case OUT_DATREG: {
            uint32_t data = READ_BYTE32();
            uint8_t reg = READ_BYTE();
            outx(data, GET_REGISTER32(reg));
            break;
        }
        default:
            printf("Instruction attempted to use a mode that doesn't exist! (code: 0x%02x)\n", prefix.mode);
            exit(1);
            return;
    }
}
