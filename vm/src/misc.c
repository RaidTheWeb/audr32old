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
    printf("doing mov instruction!\n");
    switch(prefix.mode) {
        case MOV_REGDAT: {
            uint8_t reg = READ_BYTE();
            registeruni_t reguni;
            reguni.u32 = READ_BYTE32();
            SET_REGISTER(reg, reguni);
            printf("^ REG DAT (0x%02x, 0x%08x)\n", reg, reguni.u32);
            break;
        }
        case MOV_REGREG: {
            uint8_t dest = READ_BYTE();
            uint8_t src = READ_BYTE();
            registeruni_t reguni;
            reguni.u32 = GET_REGISTER32(src);
            SET_REGISTER(dest, reguni);
            printf("^ REG REG (0x%02x, 0x%02x)\n", dest, src);
            break;
        }
        case MOV_REGPTR: {
            uint8_t reg = READ_BYTE();
            ptr_t pointer = READ_PTR();
            registeruni_t reguni;
            reguni.u32 = GET_PTR(pointer);
            SET_REGISTER(reg, reguni);
            printf("^ REG PTR (0x%02x, 0x%08x)\n", reg, pointer.addr);
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

/*#define RDTSC_REG 0x00
#define RDTSC_PTR 0x01

void dordtsc(opcodepre_t prefix) {
    switch(prefix.mode) {
        case RDTSC_REG: {
            uint8_t reg = READ_BYTE();
            registeruni_t reguni;
            reguni.u32 = vm.tsc;
            SET_REGISTER(reg, reguni);
            break;
        }
        case RDTSC_PTR: {
            ptr_t pointer = READ_PTR();
            switch(pointer.ptrmode) {
                case PTR8:
                    *(pointer.ptrv.u8) = (uint8_t)vm.tsc;
                    break;
                case PTR16:
                    *(pointer.ptrv.u16) = (uint16_t)vm.tsc;
                    break;
                case PTR32:
                    *(pointer.ptrv.u32) = vm.tsc;
                    break;
            }
            break;
        }
        default:
            printf("Instruction attempted to use a mode that doesn't exist! (code: 0x%02x)\n", prefix.mode);
            exit(1);
            return;
    }
}*/

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
/*
//        LOC - CHECK
#define JNZ_REGREG 0x00
#define JNZ_REGPTR 0x01
#define JNZ_REGDAT 0x02
#define JNZ_PTRREG 0x03
#define JNZ_PTRPTR 0x04
#define JNZ_PTRDAT 0x05
#define JNZ_DATREG 0x06
#define JNZ_DATPTR 0x07
#define JNZ_DATDAT 0x08*/

//          LOC
#define JNZ_REG 0x00
#define JNZ_PTR 0x01
#define JNZ_DAT 0x02

void dojnz(opcodepre_t prefix) {
    switch(prefix.mode) {
        /*case JNZ_REGREG: {
            uint8_t location = READ_BYTE();
            uint8_t check = READ_BYTE();
            if(GET_REGISTER32(check) != 0)
                vm.regs[REG_IP] = GET_REGISTER32(location);
            break;
        }
        case JNZ_REGPTR: {
            uint8_t location = READ_BYTE();
            uint32_t check = GET_PTR(READ_PTR());
            if(check != 0)
                vm.regs[REG_IP] = GET_REGISTER32(location);
            break;
        }
        case JNZ_REGDAT: {
            uint8_t location = READ_BYTE();
            uint32_t check = READ_BYTE32();
            if(check != 0)
                vm.regs[REG_IP] = GET_REGISTER32(location);
            break;
        }
        case JNZ_PTRREG: {
            uint32_t location = GET_PTR(READ_PTR());
            uint8_t check = READ_BYTE();
            if(GET_REGISTER32(check) != 0)
                vm.regs[REG_IP] = location;
            break;
        }
        case JNZ_PTRPTR: {
            uint32_t location = GET_PTR(READ_PTR());
            uint32_t check = GET_PTR(READ_PTR());

            if(check != 0)
                vm.regs[REG_IP] = location;
            break;
        }
        case JNZ_PTRDAT: {
            uint32_t location = GET_PTR(READ_PTR());
            uint32_t check = READ_BYTE32();

            if(check != 0)
                vm.regs[REG_IP] = location;
            break;
        }
        case JNZ_DATDAT: {
            uint32_t location = READ_BYTE32();
            uint32_t check = READ_BYTE32();

            if(check == 0x00000000) {
                ;
            } else {
                vm.regs[REG_IP] = location;
            }
            break;
        }
        case JNZ_DATPTR: {
            uint32_t location = READ_BYTE32();
            uint32_t check = GET_PTR(READ_PTR());

            if(check != 0)
                vm.regs[REG_IP] = location;
            break;
        }
        case JNZ_DATREG: {
            uint32_t location = READ_BYTE32();
            uint8_t check = READ_BYTE();

            if(GET_REGISTER32(check) != 0)
                vm.regs[REG_IP] = location;
            break;
        }*/
        case JNZ_REG: {
            uint32_t location = GET_REGISTER32(READ_BYTE());

            if(!vm.flags[FLAG_ZF]) {
                vm.regs[REG_IP] = location;
            }
            break;
        }
        case JNZ_PTR: {
            uint32_t location = GET_PTR(READ_PTR());

            if(!vm.flags[FLAG_ZF])
                vm.regs[REG_IP] = location;
            break;
        }
        case JNZ_DAT: {
            uint32_t location = READ_BYTE32();

            if(!vm.flags[FLAG_ZF]) {
                printf("Jumping to location 0x%08x (ZF is not set)\n", location);
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

/*//        LOC - CHECK
#define JZ_REGREG 0x00
#define JZ_REGPTR 0x01
#define JZ_REGDAT 0x02
#define JZ_PTRREG 0x03
#define JZ_PTRPTR 0x04
#define JZ_PTRDAT 0x05
#define JZ_DATREG 0x06
#define JZ_DATPTR 0x07
#define JZ_DATDAT 0x08*/

//         LOC
#define JZ_REG 0x00
#define JZ_PTR 0x01
#define JZ_DAT 0x02

void dojz(opcodepre_t prefix) { 
    switch(prefix.mode) {
        /*case JZ_REGREG: {
            uint8_t location = READ_BYTE();
            uint8_t check = READ_BYTE();
            if(GET_REGISTER32(check) == 0)
                vm.regs[REG_IP] = GET_REGISTER32(location);
            break;
        }
        case JZ_REGPTR: {
            uint8_t location = READ_BYTE();
            uint32_t check = GET_PTR(READ_PTR());
            if(check == 0)
                vm.regs[REG_IP] = GET_REGISTER32(location);
            break;
        }
        case JZ_REGDAT: {
            uint8_t location = READ_BYTE();
            uint32_t check = READ_BYTE32();
            if(check == 0)
                vm.regs[REG_IP] = GET_REGISTER32(location);
            break;
        }
        case JZ_PTRREG: {
            uint32_t location = GET_PTR(READ_PTR());
            uint8_t check = READ_BYTE();
            if(GET_REGISTER32(check) == 0)
                vm.regs[REG_IP] = location;
            break;
        }
        case JZ_PTRPTR: {
            uint32_t location = GET_PTR(READ_PTR());
            uint32_t check = GET_PTR(READ_PTR());

            if(check == 0)
                vm.regs[REG_IP] = location;
            break;
        }
        case JZ_PTRDAT: {
            uint32_t location = GET_PTR(READ_PTR());
            uint32_t check = READ_BYTE32();

            if(check == 0)
                vm.regs[REG_IP] = location;
            break;
        }
        case JZ_DATDAT: {
            uint32_t location = READ_BYTE32();
            uint32_t check = READ_BYTE32();

            if(check == 0)
                vm.regs[REG_IP] = location;
            break;
        }
        case JZ_DATPTR: {
            uint32_t location = READ_BYTE32();
            uint32_t check = GET_PTR(READ_PTR());

            if(check == 0)
                vm.regs[REG_IP] = location;
            break;
        }
        case JZ_DATREG: {
            uint32_t location = READ_BYTE32();
            uint8_t check = READ_BYTE();

            if(GET_REGISTER32(check) == 0)
                vm.regs[REG_IP] = location;
            break;
        }*/
        case JZ_REG: {
            uint32_t location = GET_REGISTER32(READ_BYTE());

            if(vm.flags[FLAG_ZF])
                vm.regs[REG_IP] = location;
            break;
        }
        case JZ_PTR: {
            uint32_t location = GET_PTR(READ_PTR());

            if(vm.flags[FLAG_ZF])
                vm.regs[REG_IP] = location;
            break;
        }
        case JZ_DAT: {
            uint32_t location = READ_BYTE32();

            if(vm.flags[FLAG_ZF])
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

            if(!vm.flags[FLAG_ZF] && !vm.flags[FLAG_CF])
                vm.regs[REG_IP] = location;
            break;
        }
        case JG_PTR: {
            uint32_t location = GET_PTR(READ_PTR());

            if(!vm.flags[FLAG_ZF] && !vm.flags[FLAG_CF])
                vm.regs[REG_IP] = location;
            break;
        }
        case JG_DAT: {
            uint32_t location = READ_BYTE32();

            if(!vm.flags[FLAG_ZF] && !vm.flags[FLAG_CF])
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

            if((!vm.flags[FLAG_ZF] && !vm.flags[FLAG_CF]) || vm.flags[FLAG_ZF])
                vm.regs[REG_IP] = location;
            break;
        }
        case JGE_PTR: {
            uint32_t location = GET_PTR(READ_PTR());

            if((!vm.flags[FLAG_ZF] && !vm.flags[FLAG_CF]) || vm.flags[FLAG_ZF]) 
                vm.regs[REG_IP] = location;
            break;
        }
        case JGE_DAT: {
            uint32_t location = READ_BYTE32();

            if((!vm.flags[FLAG_ZF] && !vm.flags[FLAG_CF]) || vm.flags[FLAG_ZF])
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

            if(!vm.flags[FLAG_ZF] && vm.flags[FLAG_CF])
                vm.regs[REG_IP] = location;
            break;
        }
        case JL_PTR: {
            uint32_t location = GET_PTR(READ_PTR());

            if(!vm.flags[FLAG_ZF] && vm.flags[FLAG_CF])
                vm.regs[REG_IP] = location;
            break;
        }
        case JL_DAT: {
            uint32_t location = READ_BYTE32();

            if(!vm.flags[FLAG_ZF] && vm.flags[FLAG_CF])
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

            if((!vm.flags[FLAG_ZF] && vm.flags[FLAG_CF]) || vm.flags[FLAG_ZF])
                vm.regs[REG_IP] = location;
            break;
        }
        case JLE_PTR: {
            uint32_t location = GET_PTR(READ_PTR());

            if((!vm.flags[FLAG_ZF] && vm.flags[FLAG_CF]) || vm.flags[FLAG_ZF]) 
                vm.regs[REG_IP] = location;
            break;
        }
        case JLE_DAT: {
            uint32_t location = READ_BYTE32();

            if((!vm.flags[FLAG_ZF] && !vm.flags[FLAG_CF]) || vm.flags[FLAG_ZF])
                vm.regs[REG_IP] = location;
            break;
        }
        default:
            printf("Instruction attempted to use a mode that doesn't exist! (code: 0x%02x)\n", prefix.mode);
            exit(1);
            return;
    }
}
