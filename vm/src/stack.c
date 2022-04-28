#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "vm.h"
#include "common.h"

static void PUSH(uint32_t value) {
    *vm.stacktop = value;
    vm.stacktop++;
    vm.regs[REG_SP]++; 
}

static uint32_t POP() {
    vm.stacktop--;
    vm.regs[REG_SP]--;
    return *vm.stacktop;
}

#define POP_REG 0x00
#define POP_PTR 0x01

void dopop(opcodepre_t prefix) {
    switch(prefix.mode) {
        case POP_REG: {
            uint8_t reg = READ_BYTE();
            vm.regs[reg] = POP();
            break;
        }
        case POP_PTR: {
            ptr_t pointer = READ_PTR();
            
            switch(pointer.ptrmode) {
                case PTR8:
                case PTRREG8:
//                    *(pointer.ptrv.u8) = ensurelittle32(POP());
                    SET_PTR(pointer, POP());
                    break;
                case PTR16:
                case PTRREG16:
//                    *(pointer.ptrv.u16) = ensurelittle32(POP());
                    SET_PTR(pointer, POP());
                    break;
                case PTR32:
                case PTRREG32:
//                    *(pointer.ptrv.u32) = ensurelittle32(POP());
                    SET_PTR(pointer, POP());
                    break;
            }
            break;
        }
        default:
            printf("Instruction attempted to use a mode that doesn't exist! (code: 0x%02x)\n", prefix.mode);
            exit(1);
            return;
    }
}

#define PUSH_REG 0x00
#define PUSH_PTR 0x01
#define PUSH_DAT 0x02

void dopush(opcodepre_t prefix) {
    switch(prefix.mode) {
        case PUSH_REG: {
            uint8_t reg = READ_BYTE();
            PUSH(GET_REGISTER32(reg));
            break;
        }
        case PUSH_PTR: {
            uint32_t pointer = GET_PTR(READ_PTR());
            PUSH(pointer);
            break;
        }
        case PUSH_DAT: {
            PUSH(READ_BYTE32());
            break;
        }
        default:
            printf("Instruction attempted to use a mode that doesn't exist! (code: 0x%02x)\n", prefix.mode);
            exit(1);
            return;
    }
}
