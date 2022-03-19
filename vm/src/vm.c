#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "vm.h"
#include "common.h"
#include "interrupts.h"

struct VM vm;

#define TICKSPEED 1 // MS (1000Hz)

#define HALTOP 0x34
#define NORMOP 0x35

void SET_REGISTER(uint8_t reg, registeruni_t value) {
    switch(reg) {
        /*case REG_AL:
            SET_REGISTER8(reg, value.u8);
            break;*/
        case REG_AX:
        case REG_BX:
        case REG_CX:
        case REG_DX:
        case REG_SI:
        case REG_DI:
        case REG_SP:
        case REG_BP:
        case REG_IP:
        case REG_R8:
        case REG_R9:
        case REG_R10:
        case REG_R11:
        case REG_R12:
        case REG_R13:
        case REG_R14:
        case REG_R15:
            SET_REGISTER32(reg, value.u32);
            break;
        default:
            printf("Instruction attempted to set a register that doesn't exist! (code: 0x%02x)\n", reg);
            exit(1);
            return;
    }
}

/** Misc */
void domov(opcodepre_t);
//void dordtsc(opcodepre_t);
void doint(opcodepre_t);
void dojmp(opcodepre_t);
void dojnz(opcodepre_t);
void dojz(opcodepre_t);

/** Procedures */
void docall(opcodepre_t);
void doret(opcodepre_t);

/** I/O */
void doinx(opcodepre_t);
void dooutx(opcodepre_t);

/** Stack */
void dopop(opcodepre_t);
void dopush(opcodepre_t);

/** Arithmetic */
void doadd(opcodepre_t);
void doiadd(opcodepre_t);
void dosub(opcodepre_t);
void doisub(opcodepre_t);
void dodiv(opcodepre_t);
void doidiv(opcodepre_t);
void domul(opcodepre_t);
void doimul(opcodepre_t);

void doinc(opcodepre_t);
void dodec(opcodepre_t);
void docmp(opcodepre_t);
void doand(opcodepre_t);
void doshl(opcodepre_t);
void doshr(opcodepre_t);
void doxor(opcodepre_t);
void door(opcodepre_t);
void donot(opcodepre_t);

static int doopcode(opcodepre_t opcodeprefix) {
    switch(opcodeprefix.instruction) {
        /** Misc */
        case OP_HALT:
            return HALTOP;
        case OP_NOOP:
            return NORMOP;
        case OP_MOV:
            domov(opcodeprefix);
            return NORMOP;
        case OP_INT:
            doint(opcodeprefix);
            return NORMOP;
        /*case OP_RDTSC:
            dordtsc(opcodeprefix);
            return NORMOP;*/
        case OP_JMP:
            dojmp(opcodeprefix);
            return NORMOP;
        case OP_JNZ:
            dojnz(opcodeprefix);
            return NORMOP;
        case OP_JZ:
            dojz(opcodeprefix);
            return NORMOP;

        /** Procedures */
        case OP_CALL: {
            uint32_t loc = READ_BYTE32();
            *vm.stacktop = vm.regs[REG_IP];
            vm.stacktop++;

            vm.regs[REG_IP] = loc;
            return NORMOP;
        }
        case OP_RET: {
            uint32_t loc;
            vm.stacktop--;
            vm.regs[REG_IP] = *vm.stacktop;
            return NORMOP;
        }

        /** I/O */
        case OP_INX:
            doinx(opcodeprefix);
            return NORMOP;
        case OP_OUTX:
            dooutx(opcodeprefix);
            return NORMOP;
        
        /** Stack */
        case OP_POP:
            dopop(opcodeprefix);
            return NORMOP;
        case OP_PUSH:
            dopush(opcodeprefix);
            return NORMOP;
        
        /** Arithmetic */
        case OP_ADD:
            doadd(opcodeprefix);
            return NORMOP;
        case OP_IADD:
            return NORMOP;
        case OP_SUB:
            dosub(opcodeprefix);
            return NORMOP;
        case OP_ISUB:
            return NORMOP;
        case OP_DIV:
            dodiv(opcodeprefix);
            return NORMOP;
        case OP_IDIV:
            return NORMOP;
        case OP_MUL:
            domul(opcodeprefix);
            return NORMOP;
        case OP_IMUL:
            return NORMOP;
        case OP_INC:
            doinc(opcodeprefix);
            return NORMOP;
        case OP_DEC:
            dodec(opcodeprefix);
            return NORMOP;
        case OP_CMP:
            docmp(opcodeprefix);
            return NORMOP;
        case OP_AND:
            doand(opcodeprefix);
            return NORMOP;
        case OP_SHL:
            doshl(opcodeprefix);
            return NORMOP;
        case OP_SHR:
            doshr(opcodeprefix);
            return NORMOP;
        case OP_XOR:
            doxor(opcodeprefix);
            return NORMOP;
        case OP_OR:
            door(opcodeprefix);
            return NORMOP;
        case OP_NOT:
            donot(opcodeprefix);
            return NORMOP;
    }
    return NORMOP;
}

interrupt_t interrupt_read(void);

void screen_set_title(const char *);

static void halt() {
    screen_set_title("VM - Execution Suspended");
    for(;;) {
        FOR_DEVICES(id, dev) {
            if(dev->id == 0 || !dev->tick) continue;
            dev->tick(dev);
        }
        interrupt_t inter = interrupt_read();
        if(inter.busid != 0) break;
        vm.tsc++;
    }
    screen_set_title("VM");
}

static void ceaseop() {
    for(;;);
}


void interrupt_init(void);
void screen_init(void);
void kbd_init(void);

void wait_until_triggered(uint16_t, uint16_t);

void run(uint8_t *source, size_t datalength) {
    size_t k = 0; // should be the offset of which to load the image in memory.
    for(size_t i = 0; i < datalength; i++) { // start after magic
        vm.memory[k++] = source[i]; // insert source data into memory
    }

    vm.datalength = datalength;
    //vm.ip = 0; // should point to the offset of which to load the image in memory.
    vm.regs[REG_IP] = 0;
    vm.stacktop = vm.stack; // make stacktop point to stack
    vm.tsc = 0; // ensure ticks are at zero
    srand(time(NULL));

    interrupt_init();
    screen_init();
    kbd_init();

    // initialise devices
    FOR_DEVICES(id, dev) {
        if(dev->id == 0 || !dev->tick) continue;
        dev->tick(dev);
    }

    for(;;) {
        //if(vm.regs[REG_IP] >= vm.datalength) ceaseop();
        opcodepre_t opcodeprefix;
        opcodeprefix.instruction = READ_BYTE();
        opcodeprefix.mode = READ_BYTE();
        int result = doopcode(opcodeprefix);
        if(result == HALTOP) halt();
        //wait_until_triggered(0x0001, 0x0001);
        vm.tsc++; // increase tick count
        msleep(TICKSPEED); // Sleep required tickspeed
        FOR_DEVICES(id, dev) {
            if(dev->id == 0 || !dev->tick) continue;
            dev->tick(dev);
        }
        
        printf("+==== Register Dump ====+\n");
        printf("AX: 0x%08x (%u)\n", GET_REGISTER32(REG_AX), GET_REGISTER32(REG_AX));
        printf("BX: 0x%08x (%u)\n", GET_REGISTER32(REG_BX), GET_REGISTER32(REG_BX));
        printf("CX: 0x%08x (%u)\n", GET_REGISTER32(REG_CX), GET_REGISTER32(REG_CX));
        printf("DX: 0x%08x (%u)\n", GET_REGISTER32(REG_DX), GET_REGISTER32(REG_DX));
        printf("SI: 0x%08x (%u)\n", GET_REGISTER32(REG_SI), GET_REGISTER32(REG_SI));
        printf("DI: 0x%08x (%u)\n", GET_REGISTER32(REG_DI), GET_REGISTER32(REG_DI));
        printf("SP: 0x%08x (%u)\n", GET_REGISTER32(REG_SP), GET_REGISTER32(REG_SP));
        printf("BP: 0x%08x (%u)\n", GET_REGISTER32(REG_BP), GET_REGISTER32(REG_BP));
        printf("IP: 0x%08x (%u)\n", GET_REGISTER32(REG_IP), GET_REGISTER32(REG_IP));
        printf("R8: 0x%08x (%u)\n", GET_REGISTER32(REG_R8), GET_REGISTER32(REG_R8));
        printf("R9: 0x%08x (%u)\n", GET_REGISTER32(REG_R9), GET_REGISTER32(REG_R9));
        printf("R10: 0x%08x (%u)\n", GET_REGISTER32(REG_R10), GET_REGISTER32(REG_R10));
        printf("R11: 0x%08x (%u)\n", GET_REGISTER32(REG_R11), GET_REGISTER32(REG_R11));
        printf("R12: 0x%08x (%u)\n", GET_REGISTER32(REG_R12), GET_REGISTER32(REG_R12));
        printf("R13: 0x%08x (%u)\n", GET_REGISTER32(REG_R13), GET_REGISTER32(REG_R13));
        printf("R14: 0x%08x (%u)\n", GET_REGISTER32(REG_R14), GET_REGISTER32(REG_R14));
        printf("R15: 0x%08x (%u)\n", GET_REGISTER32(REG_R15), GET_REGISTER32(REG_R15));
        printf("TSC: 0x%08x (%u)\n", vm.tsc, vm.tsc);
        printf("+==== Register Dump ====+\n");
        
    }

    for(;;); // hang "CPU" (Out of instructions/Halted)
}
