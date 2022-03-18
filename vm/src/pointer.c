#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "vm.h"

ptr_t READ_PTR() {
    ptr_t pointer;
    pointer.ptrmode = READ_BYTE();

    switch(pointer.ptrmode) {
        case PTR8: {
            uint32_t addr = READ_BYTE32();
            pointer.ptrv.u8 = (uint8_t *)&vm.memory[addr];
            return pointer;
        }
        case PTR16: {
            uint32_t addr = READ_BYTE32();
            pointer.ptrv.u16 = (uint16_t *)&vm.memory[addr];
            return pointer;
        }
        case PTR32: {
            uint32_t addr = READ_BYTE32();
            pointer.ptrv.u32 = (uint32_t *)&vm.memory[addr];
            return pointer;
        }
        case PTRREG: {
            uint8_t reg = READ_BYTE();
            pointer.ptrv.u32 = (uint32_t *)&vm.memory[vm.regs[reg]];
            return pointer;
        }
        default:
            printf("Pointer attempted to use a mode that doesn't exist (code: 0x%02x)\n", pointer.ptrmode);
            exit(1);
            return pointer;
    }
}

uint32_t GET_PTR(ptr_t pointer) {
    switch(pointer.ptrmode) {
        case PTR8:
            return ensurebig32((uint32_t)*(pointer.ptrv.u8));
        case PTR16:
            return ensurebig32((uint32_t)*(pointer.ptrv.u16));
        case PTRREG:
        case PTR32:
            return ensurebig32((uint32_t)*(pointer.ptrv.u32));
        default:
            printf("Pointer attempted to use a mode that doesn't exist (code: 0x%02x)\n", pointer.ptrmode);
            exit(1);
            return 0;
    }
}
