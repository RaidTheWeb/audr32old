#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include <SDL2/SDL.h>

#include "bus.h"
#include "clock.h"
#include "disk.h"
#include "serial.h"
#include "vm.h"
#include "pmu.h"
#include "common.h"
#include "interrupts.h"
#include "ram.h"

struct VM vm;

#define CLOCKSPEED 25000000 // 25MHz
#define FPS 60
#define TPF 1
#define TICKSPEED (FPS * TPF) // TPS

#define HALTOP 0x34
#define NORMOP 0x35

static char *instructions[] = {
    [OP_NOOP] = "noop",
    [OP_HALT] = "halt",
    [OP_MOV] = "mov",
    [OP_INT] = "int",
    [OP_JMP] = "jmp",
    [OP_JNZ] = "jnz",
    [OP_JZ] = "jz",
    [OP_CALL] = "call",
    [OP_RET] = "ret",
    [OP_INX] = "inx",
    [OP_OUTX] = "outx",
    [OP_POP] = "pop",
    [OP_PUSH] = "push",
    [OP_ADD] = "add",
    [OP_SUB] = "sub",
    [OP_DIV] = "div",
    [OP_MUL] = "mul",
    [OP_INC] = "inc",
    [OP_DEC] = "dec",
    [OP_CMP] = "cmp",
    [OP_AND] = "and",
    [OP_SHL] = "shl",
    [OP_SHR] = "shr",
    [OP_XOR] = "xor",
    [OP_OR] = "or",
    [OP_NOT] = "not",
    [OP_JL] = "jl",
    [OP_JLE] = "jle",
    [OP_JG] = "jg",
    [OP_JGE] = "jge",
    [OP_SETEQ] = "seteq",
    [OP_SETNE] = "setne",
    [OP_SETLT] = "setlt",
    [OP_SETGT] = "setgt",
    [OP_SETLE] = "setle",
    [OP_SETGE] = "setge",
    [OP_LEA] = "lea",
    [OP_NEG] = "neg",
    [OP_TEST] = "test",
    [OP_CLD] = "cld",
    [OP_LODSB] = "lodsb",
    [OP_LODSW] = "lodsw",
    [OP_LODSD] = "lodsd",
    [OP_LOOP] = "loop",
    [OP_PUSHA] = "pusha"
};

static char *exceptionnames[] = {
    [EXC_BUSERROR] = "BUSERROR",
    [EXC_BADADDR] = "BADADDR",
    [EXC_BADINST] = "BADINST"
};

static char *resolveinstruction(uint8_t instruction) {
    return instructions[instruction];
}

uint8_t GET_FLAG(uint8_t flag) {
    return (vm.flags >> flag) & 0x1;
}

void SET_FLAG(uint8_t flag, uint8_t value) {
    vm.flags ^= (-value ^ vm.flags) & (0x1 << flag); 
}

void SET_REGISTER(uint8_t reg, registeruni_t value) {
    switch(reg) {
        case REG_AX:
        case REG_BX:
        case REG_CX:
        case REG_DX:
        case REG_SI:
        case REG_DI:
        case REG_SP:
        case REG_BP:
        case REG_IP:
        case REG_R0:
        case REG_R1:
        case REG_R2:
        case REG_R3:
        case REG_R4:
        case REG_R5:
        case REG_R6:
        case REG_R7:
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

void audr32_exception(int exce) {
    if(vm.curexception) {
        printf("Double exception occurred! (prev: %s, cur: %s)\n", exceptionnames[vm.curexception], exceptionnames[exce]);
        exit(1);
    }

    vm.curexception = exce;
}

/** Misc */
void domov(opcodepre_t);
//void dordtsc(opcodepre_t);
void doint(opcodepre_t);
void dojmp(opcodepre_t);
void dojnz(opcodepre_t);
void dojz(opcodepre_t);
void dojg(opcodepre_t);
void dojge(opcodepre_t);
void dojl(opcodepre_t);
void dojle(opcodepre_t);
void doseteq(opcodepre_t);
void dosetne(opcodepre_t);
void dosetlt(opcodepre_t);
void dosetgt(opcodepre_t);
void dosetle(opcodepre_t);
void dosetge(opcodepre_t);
void dolea(opcodepre_t);

/** Procedures */
void docall(opcodepre_t);
void doret(opcodepre_t);

/** I/O */
void doinx(opcodepre_t);
void dooutx(opcodepre_t);

/** Stack */
void dopop(opcodepre_t);
void dopush(opcodepre_t);
void dopusha(opcodepre_t);
void dopopa(opcodepre_t);

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
void doneg(opcodepre_t);
void dotest(opcodepre_t);
void doloop(opcodepre_t);

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
        case OP_JMP:
            dojmp(opcodeprefix);
            return NORMOP;
        case OP_JNZ:
            dojnz(opcodeprefix);
            return NORMOP;
        case OP_JZ:
            dojz(opcodeprefix);
            return NORMOP;
        case OP_JL:
            dojl(opcodeprefix);
            return NORMOP;
        case OP_JLE:
            dojle(opcodeprefix);
            return NORMOP;
        case OP_JG:
            dojg(opcodeprefix);
            return NORMOP;
        case OP_JGE:
            dojge(opcodeprefix);
            return NORMOP;
        case OP_SETEQ:
            doseteq(opcodeprefix);
            return NORMOP;
        case OP_SETNE:
            dosetne(opcodeprefix);
            return NORMOP;
        case OP_SETLT:
            dosetlt(opcodeprefix);
            return NORMOP;
        case OP_SETGT:
            dosetgt(opcodeprefix);
            return NORMOP;
        case OP_SETLE:
            dosetle(opcodeprefix);
            return NORMOP;
        case OP_SETGE:
            dosetge(opcodeprefix);
            return NORMOP;
        case OP_LEA:
            dolea(opcodeprefix);
            return NORMOP;

        /** Procedures */
        case OP_CALL: {
            uint32_t loc = 0;
            if(opcodeprefix.mode == 0x00) loc = READ_BYTE32();
            else if(opcodeprefix.mode == 0x01) loc = GET_PTR(READ_PTR());
            else return NORMOP; // TODO: proper mode error handling
            
            vm.regs[REG_SP] -= 4;
            ptr_t pointer = {
                .addr = vm.regs[REG_SP],
                .ptrmode = 0x03
            };
            SET_PTR(pointer, vm.regs[REG_IP]);
            
            //*vm.stacktop = vm.regs[REG_IP];
            //vm.stacktop++;

            vm.regs[REG_IP] = loc;
            return NORMOP;
        }
        case OP_RET: {
            uint32_t loc;

            ptr_t pointer = {
                .addr = vm.regs[REG_SP],
                .ptrmode = 0x03
            };
            vm.regs[REG_SP] += 4;
            // printf("ret 0x%08x(sp), 0x%08x\n", vm.regs[REG_SP], GET_PTR(pointer));
            loc = GET_PTR(pointer);
            vm.regs[REG_IP] = loc;
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
        case OP_PUSHA:
            dopusha(opcodeprefix);
            return NORMOP;
        case OP_POPA:
            dopopa(opcodeprefix);
            return NORMOP;
        
        /** Arithmetic */
        case OP_ADD:
            doadd(opcodeprefix);
            return NORMOP; 
        case OP_SUB:
            dosub(opcodeprefix);
            return NORMOP;
        case OP_DIV:
            dodiv(opcodeprefix);
            return NORMOP; 
        case OP_MUL:
            domul(opcodeprefix);
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
        case OP_NEG:
            doneg(opcodeprefix);
            return NORMOP;
        case OP_TEST:
            dotest(opcodeprefix);
            return NORMOP;
        case OP_CLD:
            SET_FLAG(FLAG_DF, 0);
            return NORMOP;
        case OP_LODSB:
            vm.regs[REG_AX] = vm.regs[REG_SI];
            if(GET_FLAG(FLAG_DF) == 0) vm.regs[REG_SI] = vm.regs[REG_SI] + 1;
            else vm.regs[REG_SI] = vm.regs[REG_SI] - 1;
            return NORMOP;
        case OP_LODSW:
            vm.regs[REG_AX] = vm.regs[REG_SI];
            if(GET_FLAG(FLAG_DF) == 0) vm.regs[REG_SI] = vm.regs[REG_SI] + 2;
            else vm.regs[REG_SI] = vm.regs[REG_SI] - 2;
            return NORMOP;
        case OP_LODSD:
            if(GET_FLAG(FLAG_DF) == 0) vm.regs[REG_SI] = vm.regs[REG_SI] + 4;
            else vm.regs[REG_SI] = vm.regs[REG_SI] - 4;
            return NORMOP;
        case OP_LOOP:
            doloop(opcodeprefix);
            return NORMOP;

        default: // invalid instruction
            audr32_exception(EXC_BADINST);
            return NORMOP;
        
    }
    return NORMOP;
}
uint32_t cpu_readbyte(uint32_t addr) {
    uint32_t busval;
    if(read_bus(addr, BUS_BYTE, &busval) == BUS_ERR) {
        // fprintf(stderr, "Bad read (8-bit) from 0x%08x", addr);
        // exit(1);
        audr32_exception(EXC_BADADDR);
    }

    return busval;
}

uint32_t cpu_readword(uint32_t addr) {
    uint32_t busval;

    if(read_bus(addr, BUS_WORD, &busval) == BUS_ERR) {
        // fprintf(stderr, "Bad read (16-bit) from 0x%08x", addr);
        // exit(1);
        audr32_exception(EXC_BADADDR);
    }

    return busval;
}

uint32_t cpu_readdword(uint32_t addr) {
    uint32_t busval;

    if(read_bus(addr, BUS_DWORD, &busval) == BUS_ERR) {
        // fprintf(stderr, "Bad read (32-bit) from 0x%08x", addr);
        // exit(1);
        audr32_exception(EXC_BADADDR);
    }

    return busval;
}

void cpu_writebyte(uint32_t addr, uint32_t value) {
    if(write_bus(addr, BUS_BYTE, value) == BUS_ERR) {
        // fprintf(stderr, "Bad write (8-bit) to 0x%08x", addr);
        // exit(1);
        audr32_exception(EXC_BADADDR);
    }
}

void cpu_writeword(uint32_t addr, uint32_t value) {

    if(write_bus(addr, BUS_WORD, value) == BUS_ERR) {
        // fprintf(stderr, "Bad write (16-bit) to 0x%08x", addr);
        // exit(1);
        audr32_exception(EXC_BADADDR);
    }
}

void cpu_writedword(uint32_t addr, uint32_t value) {

    if(write_bus(addr, BUS_DWORD, value) == BUS_ERR) {
        // fprintf(stderr, "Bad write (32-bit) to 0x%08x", addr);
        // exit(1);
        audr32_exception(EXC_BADADDR);
    }
}

void interrupt_init(void);
void screen_init(void);
void kbd_init(void);

void wait_until_triggered(uint16_t, uint16_t);

void run(uint32_t ramsize) {
    load_rom(optbootrom);
    init_bus(ramsize);
    init_serial();
    init_clock();
    init_drive();
    init_pmu(); 
    for(int i = 0; i < vm.drivenum; i++) {
        if(!drive_attachimage(vm.drives[i])) {
            exit(1);
        }
    }

    if(optramimage != NULL) {
        load_ram(optramimage);
    }

    vm.regs[REG_IP] = optip; // should point to the offset of which to load the image in memory. 
    vm.regs[REG_SP] = ADDR_STACKRAMEND;
    srand(time(NULL)); // seed the random number generator

    interrupt_init();
    screen_init();
    kbd_init();

    // initialise devices
    FOR_DEVICES(id, dev) {
        if(dev->id == 0 || !dev->tick) continue;
        dev->tick(dev);
    }
    
    int ticks = 0;
    uint32_t tick_start = SDL_GetTicks();
    uint32_t tick_end = SDL_GetTicks();
    vm.running = 1;

    while(vm.running) {
        int dueticks = SDL_GetTicks() - tick_start;

        tick_start = SDL_GetTicks();

        if(!dueticks)
            dueticks = 1;

        int cyclespertick = CLOCKSPEED/TICKSPEED/dueticks;
        int extracycles = CLOCKSPEED/TICKSPEED - (cyclespertick*dueticks);

        for(size_t i = 0; i < dueticks; i++) {
            int cyclesleft = cyclespertick;

            if(i == dueticks - 1)
                cyclesleft += extracycles;

            void serial_tick(uint32_t dt);
            void clock_tick(uint32_t dt);
            serial_tick(1);
            clock_tick(1);
            
            while(cyclesleft > 0) {
                if(vm.halted) {
                    if(vm.curexception || vm.interruptpending) {
                        vm.halted = 0;
                    } else {
                        continue;
                    }
                }
                cyclesleft--;
                opcodepre_t opcodeprefix;
                opcodeprefix.instruction = READ_BYTE();
                opcodeprefix.mode = READ_BYTE();
                int result = doopcode(opcodeprefix);
                if(result == HALTOP) vm.halted = 1;
            }

        }

        printf("bx: 0x%08x\n", vm.regs[REG_BX]);

        if((ticks%TPF) == 0) {
            void screen_blit(device_t *dev);
            screen_blit(&vm.devices[0x0003]);
        }

        FOR_DEVICES(id, dev) {
            if(dev->id == 0 || !dev->tick || dev->set != 1) continue;
            dev->tick(dev);
        }

        ticks++;

        tick_end = SDL_GetTicks();
        int delay = 1000/TICKSPEED - (tick_end - tick_start);
        if(delay > 0) {
            SDL_Delay(delay);
        }
    }
}
