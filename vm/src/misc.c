#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "vm.h"
#include "common.h"

//        DEST - SRC
#define MOV_REGREG 0x00
#define MOV_REGPTR 0x01
#define MOV_REGDAT 0x02
#define MOV_PTRREG 0x03
#define MOV_PTRDAT 0x04
#define MOV_PTRPTR 0x05

void domov(opcodepre_t prefix) {
    
    switch(prefix.mode) {
        case MOV_REGDAT: {
            uint8_t reg = READ_BYTE();
            vm.regs[reg] = READ_BYTE32();
            // printf("mov 0x%02x 0x%08x\n", reg, vm.regs[reg]);
            break;
        }
        case MOV_REGREG: {
            uint8_t dest = READ_BYTE();
            uint8_t src = READ_BYTE();
            vm.regs[dest] = vm.regs[src];
            // printf("mov 0x%02x 0x%02x 0x%08x\n", dest, src, vm.regs[dest]);
            break;
        }
        case MOV_REGPTR: {
            uint8_t reg = READ_BYTE();
            ptr_t pointer = READ_PTR();
            vm.regs[reg] = GET_PTR(pointer);
            // printf("mov 0x%02x 0x%08x\n", reg, vm.regs[reg]);
            break;
        }
        case MOV_PTRREG: {
            ptr_t pointer = READ_PTR();
            uint8_t reg = READ_BYTE();
            SET_PTR(pointer, GET_REGISTER32(reg));
            break;
        }
        case MOV_PTRDAT: {
            ptr_t pointer = READ_PTR();
            uint32_t data = READ_BYTE32();
            SET_PTR(pointer, data);
            break;
        }
        case MOV_PTRPTR: {
            ptr_t dest = READ_PTR();
            ptr_t src = READ_PTR();
            SET_PTR(dest, GET_PTR(src)); 
            break;
        }
        default:
            printf("Instruction attempted to use a mode that doesn't exist! (code: 0x%02x)\n", prefix.mode);
            exit(1);
            return;
    }
}

#define JMP_REG 0x00
#define JMP_PTR 0x01
#define JMP_DAT 0x02

void dojmp(opcodepre_t prefix) {
    switch(prefix.mode) {
        case JMP_REG: {
            uint8_t reg = READ_BYTE();
            vm.regs[REG_IP] = GET_REGISTER32(reg);
            break;
        }
        case JMP_PTR: {
            uint32_t location = GET_PTR(READ_PTR());
            vm.regs[REG_IP] = location;
            break;
        }
        case JMP_DAT: {
            uint32_t loc = READ_BYTE32();
            vm.regs[REG_IP] = loc; 
            break;
        }
        default:
            printf("Instruction attempted to use a mode that doesn't exist! (code: 0x%02x)\n", prefix.mode);
            exit(1);
            return;
    }
}

//          LOC
#define JNZ_REG 0x00
#define JNZ_PTR 0x01
#define JNZ_DAT 0x02

void dojnz(opcodepre_t prefix) {
    switch(prefix.mode) {
        case JNZ_REG: {
            uint32_t location = GET_REGISTER32(READ_BYTE());

            if(!GET_FLAG(FLAG_ZF)) {
                vm.regs[REG_IP] = location;
            }
            break;
        }
        case JNZ_PTR: {
            uint32_t location = GET_PTR(READ_PTR());

            if(!GET_FLAG(FLAG_ZF))
                vm.regs[REG_IP] = location;
            break;
        }
        case JNZ_DAT: {
            uint32_t location = READ_BYTE32();

            if(!GET_FLAG(FLAG_ZF)) { 
                vm.regs[REG_IP] = location;
            }
            break;
        }
        default:
            printf("Instruction attempted to use a mode that doesn't exist! (code: 0x%02x)\n", prefix.mode);
            exit(1);
            return;
    }
}

//         LOC
#define JZ_REG 0x00
#define JZ_PTR 0x01
#define JZ_DAT 0x02

void dojz(opcodepre_t prefix) { 
    switch(prefix.mode) {
        case JZ_REG: {
            uint32_t location = GET_REGISTER32(READ_BYTE());

            if(GET_FLAG(FLAG_ZF))
                vm.regs[REG_IP] = location;
            break;
        }
        case JZ_PTR: {
            uint32_t location = GET_PTR(READ_PTR());

            if(GET_FLAG(FLAG_ZF))
                vm.regs[REG_IP] = location;
            break;
        }
        case JZ_DAT: {
            uint32_t location = READ_BYTE32();

            if(GET_FLAG(FLAG_ZF))
                vm.regs[REG_IP] = location;
            break;
        }
        default:
            printf("Instruction attempted to use a mode that doesn't exist! (code: 0x%02x)\n", prefix.mode);
            exit(1);
            return;
    }
}

//         LOC
#define JG_REG 0x00
#define JG_PTR 0x01
#define JG_DAT 0x02

void dojg(opcodepre_t prefix) {
    switch(prefix.mode) {
        case JG_REG: {
            uint32_t location = GET_REGISTER32(READ_BYTE());

            if((!GET_FLAG(FLAG_ZF)) && GET_FLAG(FLAG_CF))
                vm.regs[REG_IP] = location;
            break;
        }
        case JG_PTR: {
            uint32_t location = GET_PTR(READ_PTR());

            if((!GET_FLAG(FLAG_ZF)) && GET_FLAG(FLAG_CF))
                vm.regs[REG_IP] = location;
            break;
        }
        case JG_DAT: {
            uint32_t location = READ_BYTE32();
            
            if((!GET_FLAG(FLAG_ZF)) && GET_FLAG(FLAG_CF))
                vm.regs[REG_IP] = location;
            break;
        }
        default:
            printf("Instruction attempted to use a mode that doesn't exist! (code: 0x%02x)\n", prefix.mode);
            exit(1);
            return;
    }
}

//         LOC
#define JGE_REG 0x00
#define JGE_PTR 0x01
#define JGE_DAT 0x02

void dojge(opcodepre_t prefix) {
    switch(prefix.mode) {
        case JGE_REG: {
            uint32_t location = GET_REGISTER32(READ_BYTE());

            if(GET_FLAG(FLAG_CF))
                vm.regs[REG_IP] = location;
            else if(GET_FLAG(FLAG_ZF))
                vm.regs[REG_IP] = location;
            break;
        }
        case JGE_PTR: {
            uint32_t location = GET_PTR(READ_PTR());

            if(GET_FLAG(FLAG_CF))
                vm.regs[REG_IP] = location;
            else if(GET_FLAG(FLAG_ZF))
                vm.regs[REG_IP] = location;
            break;
        }
        case JGE_DAT: {
            uint32_t location = READ_BYTE32(); 
            if(GET_FLAG(FLAG_CF))
                vm.regs[REG_IP] = location;
            else if(GET_FLAG(FLAG_ZF))
                vm.regs[REG_IP] = location;
            break;
        }
        default:
            printf("Instruction attempted to use a mode that doesn't exist! (code: 0x%02x)\n", prefix.mode);
            exit(1);
            return;
    }
}

//         LOC
#define JL_REG 0x00
#define JL_PTR 0x01
#define JL_DAT 0x02

void dojl(opcodepre_t prefix) {
    switch(prefix.mode) {
        case JL_REG: {
            uint32_t location = GET_REGISTER32(READ_BYTE());
            
            
            if((!GET_FLAG(FLAG_ZF)) && (!GET_FLAG(FLAG_CF)))
                vm.regs[REG_IP] = location;
            break;
        }
        case JL_PTR: {
            uint32_t location = GET_PTR(READ_PTR());

            if((!GET_FLAG(FLAG_ZF)) && (!GET_FLAG(FLAG_CF)))
                vm.regs[REG_IP] = location;
            break;
        }
        case JL_DAT: {
            uint32_t location = READ_BYTE32();

            
            if((!GET_FLAG(FLAG_ZF)) && (!GET_FLAG(FLAG_CF)))
                vm.regs[REG_IP] = location;
            break;
        }
        default:
            printf("Instruction attempted to use a mode that doesn't exist! (code: 0x%02x)\n", prefix.mode);
            exit(1);
            return;
    }
}

//         LOC
#define JLE_REG 0x00
#define JLE_PTR 0x01
#define JLE_DAT 0x02

void dojle(opcodepre_t prefix) { 
    switch(prefix.mode) {
        case JLE_REG: {
            uint32_t location = GET_REGISTER32(READ_BYTE());
            
            if((!GET_FLAG(FLAG_CF)) || GET_FLAG(FLAG_ZF))
                vm.regs[REG_IP] = location;
            break;
        }
        case JLE_PTR: {
            uint32_t location = GET_PTR(READ_PTR());
            
            if((!GET_FLAG(FLAG_CF)) || GET_FLAG(FLAG_ZF)) 
                vm.regs[REG_IP] = location;
            break;
        }
        case JLE_DAT: {
            uint32_t location = READ_BYTE32();
            
            if((!GET_FLAG(FLAG_CF)) || GET_FLAG(FLAG_ZF))
                vm.regs[REG_IP] = location;
            break;
        }
        default:
            printf("Instruction attempted to use a mode that doesn't exist! (code: 0x%02x)\n", prefix.mode);
            exit(1);
            return;
    }
}

#define LEA_REGPTR 0x00
#define LEA_PTRPTR 0x01

void dolea(opcodepre_t prefix) {
    switch(prefix.mode) {
        case LEA_REGPTR: {
            uint8_t dest = READ_BYTE();
            ptr_t src = READ_PTR();

            vm.regs[dest] = src.addr;
            break;
        }
        case LEA_PTRPTR: {
            ptr_t dest = READ_PTR();
            ptr_t src = READ_PTR();
            
            SET_PTR(dest, src.addr);
            break;
        } 
        default:
            printf("Instruction attempted to use a mode that doesn't exist! (code: 0x%02x)\n", prefix.mode);
            exit(1);
            return;
    }
}

#define LOOP_REG 0x00
#define LOOP_PTR 0x01
#define LOOP_DAT 0x02

void doloop(opcodepre_t prefix) {
    uint32_t destination;
    switch(prefix.mode) {
        case LOOP_REG: {
            uint8_t reg = READ_BYTE();
            destination = vm.regs[reg];
            break;
        }
        case LOOP_PTR: {
            uint32_t location = GET_PTR(READ_PTR());
            destination = location;
            break;
        }
        case LOOP_DAT: {
            uint32_t loc = READ_BYTE32();
            destination = loc;
            break;
        }
        default:
            printf("Instruction attempted to use a mode that doesn't exist! (code: 0x%02x)\n", prefix.mode);
            exit(1);
            return;
    }

    int count = vm.regs[REG_CX] - 1;
    int branchcond = 0;

    if(count != 0) branchcond = 1;
    else branchcond = 0;

    if(branchcond == 1) {
        vm.regs[REG_IP] = destination;
    }
}
