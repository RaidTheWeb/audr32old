#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "vm.h"
#include "common.h"

static void PUSH(uint32_t value) {
    vm.regs[REG_SP] -= 4;
    ptr_t pointer = {
        .addr = vm.regs[REG_SP],
        .ptrmode = 0x03
    };
    SET_PTR(pointer, value);
}

static uint32_t POP() {
    
    ptr_t pointer = {
        .addr = vm.regs[REG_SP],
        .ptrmode = 0x03
    };
    vm.regs[REG_SP] += 4;
    return GET_PTR(pointer);
}

void dopusha(opcodepre_t prefix) {
    uint32_t temp = vm.regs[REG_SP];
    PUSH(vm.regs[REG_AX]);
    PUSH(vm.regs[REG_CX]);
    PUSH(vm.regs[REG_DX]);
    PUSH(vm.regs[REG_BX]);
    PUSH(temp);
    PUSH(vm.regs[REG_BP]);
    PUSH(vm.regs[REG_SI]);
    PUSH(vm.regs[REG_DI]);
}

void dopopa(opcodepre_t prefix) {
    vm.regs[REG_DI] = POP();
    vm.regs[REG_SI] = POP();
    vm.regs[REG_BP] = POP();
    vm.regs[REG_SP] += 4; // skip stack pointer push for some reason (https://www.felixcloutier.com/x86/popa:popad.html)
    vm.regs[REG_BX] = POP();
    vm.regs[REG_DX] = POP();
    vm.regs[REG_CX] = POP();
    vm.regs[REG_AX] = POP();
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
