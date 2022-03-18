#ifndef __VM_H__
#define __VM_H__

#include <stdint.h>
#include <stddef.h>

#include "common.h"
#include "io.h"

#define ADDR_FRAMEBUFFER 0x000B8000


#define MAX_REGS 17

// do some crazy cool shit here, maybe commit some war crimes. >//<

struct VM {
    uint8_t memory[KB * 512]; // uh oh stinky area (512KB of general purpose memory)
    size_t datalength;
    uint32_t ip;

    uint32_t stack[8192];
    uint32_t *stacktop;

    uint32_t regs[MAX_REGS + 1]; // registers
    uint32_t tsc; // timestamp counter

    device_t devices[MAX_PORTS];
};

extern struct VM vm;

typedef struct {
    uint8_t instruction;
    uint8_t mode;
} opcodepre_t;

enum {
    /** Misc */
    OP_NOOP             =           0x00,           // Perform no operation
    OP_HALT             =           0x01,           // Halt operation
    OP_MOV              =           0x02,           // Move data
    OP_INT              =           0x03,           // Call interrupt
    OP_JMP              =           0x04,           // Jump
    OP_JNZ              =           0x05,           // Jump if not zero
    OP_JZ               =           0x06,           // Jump if zero

    /** Procedures */
    OP_CALL             =           0x07,           // Call a procedure
    OP_RET              =           0x08,           // Return from a procedure

    /** I/O */
    OP_INX              =           0x09,           // Grab single double word from port
    OP_OUTX             =           0x0A,           // Send single double word to port

    /** Stack */
    OP_POP              =           0x0B,           // Pop data from stack
    OP_PUSH             =           0x0C,           // Push data onto stack

    /** Arithmetic */
    OP_ADD              =           0x0D,           // Addition
    OP_IADD             =           0x0E,           // Signed Addition
    OP_SUB              =           0x0F,           // Subtraction
    OP_ISUB             =           0x10,           // Signed Subtraction
    OP_DIV              =           0x11,           // Division
    OP_IDIV             =           0x12,           // Signed Division
    OP_MUL              =           0x13,           // Multiplication
    OP_IMUL             =           0x14,           // Signed Multiplication
    OP_INC              =           0x15,           // Increment
    OP_DEC              =           0x16,           // Decrement
    OP_CMP              =           0x17,           // Compare operands
    OP_AND              =           0x18,           // Logical AND
    OP_SHL              =           0x19,           // Shift Left
    OP_SHR              =           0x1A,           // Shift Right
    OP_XOR              =           0x1B,           // Exclusive OR
    OP_OR               =           0x1C,           // Logical OR
    OP_NOT              =           0x1D            // Logical NOT
};

enum {
    REG_AX              =           0x01,           // 32 bit accumulator register
    REG_BX              =           0x02,           // 32 bit base register
    REG_CX              =           0x03,           // 32 bit counter register
    REG_DX              =           0x04,           // 32 bit data register
    REG_SI              =           0x05,           // 32 bit source index register
    REG_DI              =           0x06,           // 32 bit destination index register
    REG_SP              =           0x07,           // 32 bit stack pointer register
    REG_BP              =           0x08,           // 32 bit base pointer register
    REG_IP              =           0x09,           // 32 bit instruction pointer register
    REG_R8              =           0x0A,           // Generic 32 bit register
    REG_R9              =           0x0B,           // Generic 32 bit register
    REG_R10             =           0x0C,           // Generic 32 bit register
    REG_R11             =           0x0D,           // Generic 32 bit register
    REG_R12             =           0x0E,           // Generic 32 bit register
    REG_R13             =           0x0F,           // Generic 32 bit register
    REG_R14             =           0x10,           // Generic 32 bit register
    REG_R15             =           0x11            // Generic 32 bit register
};

#define READ_BYTE() (uint8_t)vm.memory[vm.regs[REG_IP]++]  //vm.memory[vm.ip++]
#define READ_BYTE16() (uint16_t)((READ_BYTE() << 8) | READ_BYTE())
#define READ_BYTE32() (uint32_t)((READ_BYTE16() << 16) + READ_BYTE16())
//#define READ_PTR8(addr) (uint8_t *)&vm.memory[addr];
//#define READ_PTR16(addr) (uint16_t *)&vm.memory[addr];
//#define READ_PTR32(addr) (uint32_t *)&vm.memory[addr];

#define PTR8    0x01
#define PTR16   0x02
#define PTR32   0x03
#define PTRREG  0x04

typedef struct {
    uint8_t ptrmode; // Pointer Mode (1: 8 bit, 2: 16 bit, 3: 32 bit)
    union {
        uint8_t *u8;
        uint16_t *u16;
        uint32_t *u32;
    } ptrv;
} ptr_t;

ptr_t READ_PTR(void);
void SET_PTR(ptr_t pointer);
uint32_t GET_PTR(ptr_t pointer);

typedef union {
    uint8_t u8;
    uint16_t u16;
    uint32_t u32;
} registeruni_t;

void SET_REGISTER(uint8_t reg, registeruni_t value);
#define SET_REGISTER8(reg, value) vm.regs[reg] = (uint32_t)((uint8_t)value)
#define SET_REGISTER16(reg, value) vm.regs[reg] = (uint32_t)((uint16_t)value)
#define SET_REGISTER32(reg, value) vm.regs[reg] = (uint32_t)value
#define GET_REGISTER8(reg) ((uint8_t)vm.regs[reg])
#define GET_REGISTER16(reg) ((uint16_t)vm.regs[reg])
#define GET_REGISTER32(reg) ((uint32_t)vm.regs[reg])
#define VALID_REGISTER(reg) (reg <= (MAX_REGS))

void run(uint8_t *source, size_t datalength);

#endif
