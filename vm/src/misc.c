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
            registeruni_t reguni;
            reguni.u32 = READ_BYTE32();
            SET_REGISTER(reg, reguni);
            break;
        }
        case MOV_REGREG: {
            uint8_t dest = READ_BYTE();
            uint8_t src = READ_BYTE();
            registeruni_t reguni;
            reguni.u32 = GET_REGISTER32(src);
            SET_REGISTER(dest, reguni);
            break;
        }
        case MOV_REGPTR: {
            uint8_t reg = READ_BYTE();
            ptr_t pointer = READ_PTR();
            registeruni_t reguni;
            reguni.u32 = GET_PTR(pointer);
            SET_REGISTER(reg, reguni);
            break;
        }
        case MOV_PTRREG: {
            ptr_t pointer = READ_PTR();
            uint8_t reg = READ_BYTE();
            switch(pointer.ptrmode) {
                case PTR8:
                    *(pointer.ptrv.u8) = GET_REGISTER8(reg);
                    break;
                case PTR16:
                    *(pointer.ptrv.u16) = GET_REGISTER16(reg);
                    break;
                case PTRREG:
                case PTR32:
                    *(pointer.ptrv.u32) = GET_REGISTER32(reg);
                    break;
            }
            break;
        }
        case MOV_PTRDAT: {
            ptr_t pointer = READ_PTR();
            uint32_t data = READ_BYTE32();
            switch(pointer.ptrmode) {
                case PTR8:
                    *(pointer.ptrv.u8) = (uint8_t)data;
                    break;
                case PTR16:
                    *(pointer.ptrv.u16) = (uint16_t)data;
                    break;
                case PTRREG:
                case PTR32:
                    *(pointer.ptrv.u32) = data;
                    printf("%08x, %08x\n", data, *pointer.ptrv.u32);
                    break;
            }
            break;
        }
        case MOV_PTRPTR: {
            ptr_t dest = READ_PTR();
            ptr_t src = READ_PTR();
            switch(dest.ptrmode) {
                case PTR8:
                    *(dest.ptrv.u8) = (uint8_t)GET_PTR(src);
                    break;
                case PTR16:
                    *(dest.ptrv.u16) = (uint16_t)GET_PTR(src);
                    break;
                case PTR32:
                case PTRREG:
                    *(dest.ptrv.u32) = GET_PTR(src);
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

//        LOC - CHECK
#define JNZ_REGREG 0x00
#define JNZ_REGPTR 0x01
#define JNZ_REGDAT 0x02
#define JNZ_PTRREG 0x03
#define JNZ_PTRPTR 0x04
#define JNZ_PTRDAT 0x05
#define JNZ_DATDAT 0x06
#define JNZ_DATPTR 0x07
#define JNZ_DATREG 0x08

void dojnz(opcodepre_t prefix) {
    switch(prefix.mode) {
        case JNZ_REGREG: {
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

            if(check != 0)
                vm.regs[REG_IP] = location;
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
        }
        default:
            printf("Instruction attempted to use a mode that doesn't exist! (code: 0x%02x)\n", prefix.mode);
            exit(1);
            return;
    }
}

//        LOC - CHECK
#define JZ_REGREG 0x00
#define JZ_REGPTR 0x01
#define JZ_REGDAT 0x02
#define JZ_PTRREG 0x03
#define JZ_PTRPTR 0x04
#define JZ_PTRDAT 0x05
#define JZ_DATDAT 0x06
#define JZ_DATPTR 0x07
#define JZ_DATREG 0x08

void dojz(opcodepre_t prefix) {
    switch(prefix.mode) {
        case JZ_REGREG: {
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
        }
        default:
            printf("Instruction attempted to use a mode that doesn't exist! (code: 0x%02x)\n", prefix.mode);
            exit(1);
            return;
    }
}