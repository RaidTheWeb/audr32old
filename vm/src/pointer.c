#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "vm.h"

#define GET_PTR8(pointer) (vm.memory[pointer.addr])
#define SET_PTR8(pointer, value) (vm.memory[pointer.addr] = (value))
#define GET_PTR16(pointer) ((vm.memory[pointer.addr] << 8) | vm.memory[pointer.addr + 1])
#define SET_PTR16(pointer, value) ((vm.memory[pointer.addr] = (value) >> 8)&(vm.memory[pointer.addr + 1] = (value)))
#define GET_PTR32(pointer) (((vm.memory[pointer.addr] << 8) | vm.memory[pointer.addr + 1]) << 16) + ((vm.memory[pointer.addr + 2] << 8) | vm.memory[pointer.addr + 3])
#define SET_PTR32(pointer, value) ((vm.memory[pointer.addr] = (value) >> 24)&(vm.memory[pointer.addr + 1] = (value) >> 16)&(vm.memory[pointer.addr + 2] = (value) >> 8)&(vm.memory[pointer.addr + 3] = (value)))

ptr_t READ_PTR() {
    ptr_t pointer;
    pointer.ptrmode = READ_BYTE();

    switch(pointer.ptrmode) {
        case PTR8: {
            uint32_t addr = READ_BYTE32();
            int32_t offset = READ_BYTE32();
//            pointer.ptrv.u8 = (uint8_t *)&vm.memory[addr];
            pointer.addr = addr + offset;
            break;
        }
        case PTR16: {
            uint32_t addr = READ_BYTE32();
            int32_t offset = READ_BYTE32();
//            pointer.ptrv.u16 = (uint16_t *)&vm.memory[addr];
            pointer.addr = addr + offset;
            break;
        }
        case PTR32: {
            uint32_t addr = READ_BYTE32();
            int32_t offset = READ_BYTE32();
//            pointer.ptrv.u32 = (uint32_t *)&vm.memory[addr]];
            pointer.addr = addr + offset;
            break;
        }
        case PTRREG8: {
            uint8_t reg = READ_BYTE();
            int32_t offset = READ_BYTE32();
            pointer.addr = GET_REGISTER8(reg) + offset;
            break;
        }
        case PTRREG16: {
            uint8_t reg = READ_BYTE();
            int32_t offset = READ_BYTE32();
            pointer.addr = GET_REGISTER16(reg) + offset;
            break;
        }
        case PTRREG32: {
            uint8_t reg = READ_BYTE();
            int32_t offset = READ_BYTE32();
//            pointer.ptrv.u32 = (uint32_t *)&vm.memory[vm.regs[reg]];
            pointer.addr = GET_REGISTER32(reg) + offset;
            break;
        }
        default:
            printf("Pointer attempted to use a mode that doesn't exist (code: 0x%02x)\n", pointer.ptrmode);
            exit(1);
            return pointer;
    }
    return pointer;
}

uint32_t GET_PTR(ptr_t pointer) {
    switch(pointer.ptrmode) {
        case PTRREG8:
        case PTR8:
            //return ensurebig32((uint32_t)*(pointer.ptrv.u8));
            return GET_PTR8(pointer);
        case PTRREG16:
        case PTR16:
            //return ensurebig32((uint32_t)*(pointer.ptrv.u16));
            return GET_PTR16(pointer);
        case PTRREG32:
        case PTR32:
            //return ensurebig32((uint32_t)*(pointer.ptrv.u32));
            return GET_PTR32(pointer);
        default:
            printf("Pointer attempted to use a mode that doesn't exist (code: 0x%02x)\n", pointer.ptrmode);
            exit(1);
            return 0;
    }
}

void SET_PTR(ptr_t pointer, uint32_t byproduct) {
    switch(pointer.ptrmode) {
        case PTRREG8:
        case PTR8:
            SET_PTR8(pointer, byproduct);
            break;
        case PTRREG16:
        case PTR16:
            SET_PTR16(pointer, byproduct);
            break;
        case PTRREG32:
        case PTR32:
            SET_PTR32(pointer, byproduct);
            break;
        default:
            printf("Pointer attempted to use a mode that doesn\t exist (code: 0x%02x)\n", pointer.ptrmode);
            exit(1);
            return;
    }
}
