#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "vm.h"

// fmov r0, 0.1
//      - r0 = fstore(fload(0.1(32)))
// (since data is now packed properly it is possible to move around this data at will)
// fadd r0, 0.5
//      - r0 = fstore(fadd(r0, 0.5))


float fadd(uint32_t a, uint32_t b) { // add packed data
    float af;
    memcpy(&af, &a, sizeof(uint32_t));
    float bf;
    memcpy(&bf, &b, sizeof(uint32_t));

    return af + bf;
}

float fsub(uint32_t a, uint32_t b) { // subtract packed data
    float af;
    memcpy(&af, &a, sizeof(uint32_t));
    float bf;
    memcpy(&bf, &b, sizeof(uint32_t));

    return af - bf;
}

float fdiv(uint32_t a, uint32_t b) { // divide packed data
    float af;
    memcpy(&af, &a, sizeof(uint32_t));
    float bf;
    memcpy(&bf, &b, sizeof(uint32_t));

    return af / bf;
}

float fmul(uint32_t a, uint32_t b) { // multiply packed data
    float af;
    memcpy(&af, &a, sizeof(uint32_t));
    float bf;
    memcpy(&bf, &b, sizeof(uint32_t));

    return af * bf;
}

uint32_t fstore(float a) { // store float as packed uint32_t
    uint32_t tmp;
    memcpy(&tmp, &a, sizeof(uint32_t));
    return tmp;
}

float fload(uint32_t a) { // load packed uint32_t as float
    float af;
    memcpy(&af, &a, sizeof(uint32_t));
    return af;
}


//        DEST - SRC
#define ADD_REGREG 0x00
#define ADD_REGPTR 0x01
#define ADD_REGDAT 0x02
#define ADD_PTRREG 0x03
#define ADD_PTRPTR 0x04
#define ADD_PTRDAT 0x05

void dofadd(opcodepre_t prefix) {
    switch(prefix.mode) {
        case ADD_REGREG: {
            uint8_t first = READ_BYTE();
            uint8_t second = READ_BYTE();

            vm.regs[first] = fstore(fadd(vm.regs[first], vm.regs[second]));

            // printf("add 0x%02x 0x%02x 0x%08x\n", first, second, vm.regs[first]);

            break;
        }
        case ADD_REGPTR: {
            uint8_t first = READ_BYTE();
            uint32_t second = GET_PTR(READ_PTR());

            vm.regs[first] = fstore(fadd(vm.regs[first], second)); 

            break;
        }
        case ADD_REGDAT: {
            uint8_t first = READ_BYTE();
            uint32_t second = READ_BYTE32();

            vm.regs[first] = fstore(fadd(vm.regs[first], second)); 

            // printf("add 0x%02x 0x%08x 0x%08x\n", first, second, vm.regs[first]);

            break;
        }
        case ADD_PTRPTR: {
            ptr_t first = READ_PTR();
            uint32_t second = GET_PTR(READ_PTR());
            SET_PTR(first, fstore(fadd(GET_PTR(first), second)));
            break;
        }
        case ADD_PTRREG: {
            ptr_t first = READ_PTR();
            uint8_t second = READ_BYTE();

            SET_PTR(first, fstore(fadd(GET_PTR(first), GET_REGISTER32(second))));
            break;
        }
        case ADD_PTRDAT: {
            ptr_t first = READ_PTR();
            uint32_t second = READ_BYTE32();

            SET_PTR(first, fstore(fadd(GET_PTR(first), second)));
            break;
        }
        default:
            printf("Instruction attempted to use a mode that doesn't exist! (code: 0x%02x)\n", prefix.mode);
            exit(1);
            return;
    }
}

//        DEST - SRC
#define SUB_REGREG 0x00
#define SUB_REGPTR 0x01
#define SUB_REGDAT 0x02
#define SUB_PTRPTR 0x03
#define SUB_PTRREG 0x04
#define SUB_PTRDAT 0x05

void dofsub(opcodepre_t prefix) {
    switch(prefix.mode) {
        case SUB_REGREG: {
            uint8_t first = READ_BYTE();
            uint8_t second = READ_BYTE();

            vm.regs[first] = fstore(fsub(vm.regs[first], vm.regs[second]));

            // printf("sub 0x%02x 0x%02x 0x%08x\n", first, second, vm.regs[first]);
            break;
        }
        case SUB_REGPTR: {
            uint8_t first = READ_BYTE();
            uint32_t second = GET_PTR(READ_PTR());

            vm.regs[first] = fstore(fsub(vm.regs[first], second));

            break;
        }
        case SUB_REGDAT: {
            uint8_t first = READ_BYTE();
            uint32_t second = READ_BYTE32();

            vm.regs[first] = fstore(fsub(vm.regs[first], second));
            // printf("sub 0x%02x 0x%08x 0x%08x\n", first, second, vm.regs[first]);
            break;
        }
        case SUB_PTRPTR: {
            ptr_t first = READ_PTR();
            uint32_t second = GET_PTR(READ_PTR());
            SET_PTR(first, fstore(fsub(GET_PTR(first), second)));
            break;
        }
        case SUB_PTRREG: {
            ptr_t first = READ_PTR();
            uint8_t second = READ_BYTE();

            SET_PTR(first, fstore(fsub(GET_PTR(first), vm.regs[second])));
            break;
        }
        case SUB_PTRDAT: {
            ptr_t first = READ_PTR();
            uint32_t second = READ_BYTE32();

            SET_PTR(first, fstore(fsub(GET_PTR(first), second)));
            break;
        }

        default:
            printf("Instruction attempted to use a mode that doesn't exist! (code: 0x%02x)\n", prefix.mode);
            exit(1);
            return;
    }
}


// TODO: fucking hell, negative floats? (Wayyy too annoying)

//       DEST - SRC
#define IDIV_REGREG 0x00
#define IDIV_REGPTR 0x01
#define IDIV_REGDAT 0x02
#define IDIV_PTRPTR 0x03
#define IDIV_PTRREG 0x04
#define IDIV_PTRDAT 0x05

void dofidiv(opcodepre_t prefix) {
    switch(prefix.mode) {
        case IDIV_REGREG: {
            uint8_t first = READ_BYTE();
            uint8_t second = READ_BYTE();

            vm.regs[first] = (int32_t)vm.regs[first] / (int32_t)vm.regs[second];
            break;
        }
        case IDIV_REGPTR: {
            uint8_t first = READ_BYTE();
            uint32_t second = GET_PTR(READ_PTR());

            vm.regs[first] = (int32_t)vm.regs[first] / (int32_t)second;
            break;
        }
        case IDIV_REGDAT: {
            uint8_t first = READ_BYTE();
            uint32_t second = READ_BYTE32();

            vm.regs[first] = (int32_t)vm.regs[first] / (int32_t)second;
            break;
        }
        case IDIV_PTRPTR: {
            ptr_t first = READ_PTR();
            uint32_t second = GET_PTR(READ_PTR());
            SET_PTR(first, (int32_t)GET_PTR(first) / (int32_t)second);
            break;
        }
        case IDIV_PTRREG: {
            ptr_t first = READ_PTR();
            uint8_t second = READ_BYTE();
            SET_PTR(first, (int32_t)GET_PTR(first) / (int32_t)GET_REGISTER32(second));
            break;
        }
        case IDIV_PTRDAT: {
            ptr_t first = READ_PTR();
            uint32_t second = READ_BYTE32();
            
            SET_PTR(first, (int32_t)GET_PTR(first) / (int32_t)second);
            break;
        }
        default:
            printf("Instruction attempted to use a mode that doesn't exist! (code: 0x%02x)\n", prefix.mode);
            exit(1);
            return;
    }
}

//        DEST - SRC
#define DIV_REGREG 0x00
#define DIV_REGPTR 0x01
#define DIV_REGDAT 0x02
#define DIV_PTRPTR 0x03
#define DIV_PTRREG 0x04
#define DIV_PTRDAT 0x05

void dofdiv(opcodepre_t prefix) {
    switch(prefix.mode) {
        case DIV_REGREG: {
            uint8_t first = READ_BYTE();
            uint8_t second = READ_BYTE();

            vm.regs[first] = fstore(fdiv(vm.regs[first], vm.regs[second]));
            break;
        }
        case DIV_REGPTR: {
            uint8_t first = READ_BYTE();
            uint32_t second = GET_PTR(READ_PTR());

            vm.regs[first] = fstore(fdiv(vm.regs[first], second)); 
            break;
        }
        case DIV_REGDAT: {
            uint8_t first = READ_BYTE();
            uint32_t second = READ_BYTE32();

            vm.regs[first] = fstore(fdiv(vm.regs[first], second));
            break;
        }
        case DIV_PTRPTR: {
            ptr_t first = READ_PTR();
            uint32_t second = GET_PTR(READ_PTR());
            SET_PTR(first, fdiv(GET_PTR(first), second));
            break;
        }
        case DIV_PTRREG: {
            ptr_t first = READ_PTR();
            uint8_t second = READ_BYTE();
            SET_PTR(first, fdiv(GET_PTR(first), GET_REGISTER32(second)));
            break;
        }
        case DIV_PTRDAT: {
            ptr_t first = READ_PTR();
            uint32_t second = READ_BYTE32();
            
            SET_PTR(first, fdiv(GET_PTR(first), second));
            break;
        }
        default:
            printf("Instruction attempted to use a mode that doesn't exist! (code: 0x%02x)\n", prefix.mode);
            exit(1);
            return;
    }
}

//        DEST - SRC
#define MUL_REGREG 0x00
#define MUL_REGPTR 0x01
#define MUL_REGDAT 0x02
#define MUL_PTRPTR 0x03
#define MUL_PTRREG 0x04
#define MUL_PTRDAT 0x05

void dofmul(opcodepre_t prefix) {
    switch(prefix.mode) {
        case MUL_REGREG: {
            uint8_t first = READ_BYTE();
            uint8_t second = READ_BYTE();

            vm.regs[first] = fstore(fmul(vm.regs[first], vm.regs[second]));

            break;
        }
        case MUL_REGPTR: {
            uint8_t first = READ_BYTE();
            uint32_t second = GET_PTR(READ_PTR());

            vm.regs[first] = fstore(fmul(vm.regs[first], second));

            break;
        }
        case MUL_REGDAT: {
            uint8_t first = READ_BYTE();
            uint32_t second = READ_BYTE32();

            vm.regs[first] = fstore(fmul(vm.regs[first], second));

            break;
        }
        case MUL_PTRPTR: {
            ptr_t first = READ_PTR();
            uint32_t second = GET_PTR(READ_PTR()); 
            SET_PTR(first, fstore(fmul(GET_PTR(first), second)));
            break;
        }
        case MUL_PTRREG: {
            ptr_t first = READ_PTR();
            uint8_t second = READ_BYTE();
            SET_PTR(first, GET_PTR(first) * GET_REGISTER32(second));
            break;
        }
        case MUL_PTRDAT: {
            ptr_t first = READ_PTR();
            uint32_t second = READ_BYTE32();

            SET_PTR(first, GET_PTR(first) * second);
            break;
        }

        default:
            printf("Instruction attempted to use a mode that doesn't exist! (code: 0x%02x)\n", prefix.mode);
            exit(1);
            return;
    }
}

enum {
    FPU_TRUNC, // truncate float to integer
    FPU_CONV // turn integer to float
};

static uint32_t fpu_datamod = 0;

static void fpu_writecmd(uint16_t port, uint32_t data) {
    switch(data) {
        case FPU_TRUNC:
            fpu_datamod = (uint32_t) fload(fpu_datamod); // cheat here and just use C's truncation
            break;
        case FPU_CONV:
            fpu_datamod = fstore((float) fpu_datamod); // cheat here and just use C's truncation
            break;
    }
}

static uint32_t fpu_readcmd(uint16_t port) {
    return 0;
}

static void fpu_writemod(uint16_t port, uint32_t data) {
    fpu_datamod = data;
}

static uint32_t fpu_readmod(uint16_t port) {
    return fpu_datamod;
}

void init_fpu(void) {
    vm.ports[0x90].set = 1;
    vm.ports[0x90].write = fpu_writecmd;
    vm.ports[0x90].read = fpu_readcmd;

    vm.ports[0x9A].set = 1;
    vm.ports[0x9A].write = fpu_writemod;
    vm.ports[0x9A].read = fpu_readmod;
}
