#ifndef __VM_H__
#define __VM_H__

#include <stdint.h>
#include <stddef.h>

#include "common.h"
#include "io.h"

/** Address locations (mapped on the bus at their respective locations) */
#define ADDR_BUSREGISTERS       0x000002F5 // various bits of information about devices on the bus (about 10 slots)
#define ADDR_BUSREGISTERSEND    0x000002FF
#define ADDR_FRAMEBUFFER        0x00000300
#define ADDR_FRAMEBUFFEREND     0x0012C300
#define ADDR_TEXTBUFFER         0x0012C400 // start of video text mode buffer
#define ADDR_TEXTBUFFEREND      0x0012E020
#define ADDR_STACKRAM           0x00130000
#define ADDR_STACKRAMEND        0x0032FFFB
#define ADDR_RAM                0x00330000 // RAM starts just as stack ends
#define ADDR_RAMEND             0x4032FFFB // Accounts for full 1GB maximum
#define ADDR_ROM                0x40330000 // ROM starts just as RAM ends
#define ADDR_ROMEND             0x4033FFFF // Accounts for full 64KB of allocated ROM space
#define ADDR_SECTORCACHE        0x40340000 // Block buffer (512 bytes)
#define ADDR_SECTORCACHEEND     0x40340200

#define MAX_REGS                25

struct VM {
    uint8_t *memory; // memory (ram)
   
    uint8_t rom[64 * KB]; // boot rom (64KB) 

    size_t datalength;
    uint32_t ip;

    uint8_t running;
    uint8_t halted; // are we suspending execution right now?
    uint8_t curexception; // current exception
    uint8_t interruptpending; // is an interrupt pending right now?

    uint32_t regs[MAX_REGS + 1]; // registers
    uint8_t flags; // flags

    char **drives;
    int drivenum;

    device_t devices[MAX_PORTS];
    port_t ports[MAX_PORTS];
};

extern struct VM vm;
extern uint32_t busregs[10];

extern uint32_t optip;
extern size_t optramsize;
extern char *optbootrom;
extern char *optramimage;

typedef struct {
    uint8_t instruction;
    uint8_t mode;
} opcodepre_t;

enum {
    EXC_BUSERROR        =           0x01,           // Bus operation failed
    EXC_BADADDR         =           0x02,           // Attempted to access a illegal address
    EXC_BADINST         =           0x03,           // Attempted to interpret an instruction that does not exist
    EXC_BADINT          =           0x04,           // Triggered an interrupt with no registered handler
    EXC_BADSYS          =           0x05            // Triggered an unassigned syscall
};

enum {
    FLAG_CF             =           0x00,           // Carry flag
    FLAG_ZF             =           0x01,           // Zero flag
    FLAG_SF             =           0x02,           // Sign flag
    FLAG_TF             =           0x03,           // Trap flag (Debug)
    FLAG_IF             =           0x04,           // Interrupt flag
    FLAG_DF             =           0x05,           // Direction flag
    FLAG_OF             =           0x06            // Overflow flag
};

uint8_t GET_FLAG(uint8_t flag);
void SET_FLAG(uint8_t flag, uint8_t value);

enum {
    /** Misc */
    OP_NOP              =           0x00,           // Perform no operation
    OP_HLT              =           0x01,           // Halt operation
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
    // Should be memory mapped!
    // OUTX:
    // mov [32:addrforport], data
    // INX:
    // mov dx, [32:addrforport]

    /** Stack */
    OP_POP              =           0x0B,           // Pop data from stack
    OP_PUSH             =           0x0C,           // Push data onto stack

    /** Arithmetic */
    OP_ADD              =           0x0D,           // Addition
    OP_SUB              =           0x0E,           // Subtraction
    OP_DIV              =           0x0F,           // Division
    OP_IDIV             =           0x10,           // Signed Division
    OP_MUL              =           0x11,           // Multiplication
    OP_CMP              =           0x12,           // Compare operands
    OP_AND              =           0x13,           // Logical AND
                                                    // AND without affecting operands
    OP_SHL              =           0x14,           // Shift Left
    OP_SHR              =           0x15,           // Shift Right
    OP_XOR              =           0x16,           // Exclusive OR
    OP_OR               =           0x17,           // Logical OR
    OP_NOEG             =           0x18,          // Logical NOT
                                                    // Negate

    /** Extra Misc */
    OP_JL               =           0x19,           // Jump if less-than
    OP_JLE              =           0x1A,           // Jump if less-than or equal to
    OP_JG               =           0x1B,           // Jump if greater-than
    OP_JGE              =           0x1C,           // Jump if greater-than or equal to
    OP_SET              =           0x1D,           // Set if the Zero Flag is set/Set if the Zero Flag is not set
                                                    // Set if the Zero Flag and the Carry Flag are not set
                                                    // Set if the Zero Flag is not set but the Carry Flag is
                                                    // Set if the Zero Flag is set or the Carry Flag is not set
                                                    // Set if the Zero Flag is set or the Carry Flag is set
    OP_LEA              =           0x1E,           // Load address of pointer into destination
    OP_SYSCALL          =           0x1F,           // Syscall

    /** FUNNIES */
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
    REG_R0              =           0x0A,           // Generic 32 bit register
    REG_R1              =           0x0B,           // Generic 32 bit register
    REG_R2              =           0x0C,           // Generic 32 bit register
    REG_R3              =           0x0D,           // Generic 32 bit register
    REG_R4              =           0x0E,           // Generic 32 bit register
    REG_R5              =           0x0F,           // Generic 32 bit register
    REG_R6              =           0x10,           // Generic 32 bit register
    REG_R7              =           0x11,           // Generic 32 bit register
    REG_R8              =           0x12,           // Generic 32 bit register
    REG_R9              =           0x13,           // Generic 32 bit register
    REG_R10             =           0x14,           // Generic 32 bit register
    REG_R11             =           0x15,           // Generic 32 bit register
    REG_R12             =           0x16,           // Generic 32 bit register
    REG_R13             =           0x17,           // Generic 32 bit register
    REG_R14             =           0x18,           // Generic 32 bit register
    REG_R15             =           0x19            // Generic 32 bit register
};

// Some abstractions for reading and writing from the bus
uint32_t cpu_readbyte(uint32_t addr);
uint32_t cpu_readword(uint32_t addr);
uint32_t cpu_readdword(uint32_t addr);
void cpu_writebyte(uint32_t addr, uint32_t value);
void cpu_writeword(uint32_t addr, uint32_t value);
void cpu_writedword(uint32_t addr, uint32_t value);

// Macros to both allow for backwards compat and automatically increment instruction pointer
#define READ_BYTE() (uint8_t)cpu_readbyte(vm.regs[REG_IP]++)
#define READ_BYTE16() (uint16_t)cpu_readword((vm.regs[REG_IP] += 2) - 2) // god this is so hacky it makes me want to cry
#define READ_BYTE32() (uint32_t)cpu_readdword((vm.regs[REG_IP] += 4) - 4)

#define PTR8        0x01
#define PTR16       0x02
#define PTR32       0x03
#define PTRREG8     0x04
#define PTRREG16    0x05
#define PTRREG32    0x06

typedef struct {
    uint8_t ptrmode; // Pointer Mode (1: 8 bit, 2: 16 bit, 3: 32 bit, 4: 8 bit but we mention a register, 5: 16 bit but we mention a register, 6: 32 bit but we mention a register)
    uint32_t addr; // Pointer reference address
} ptr_t;

ptr_t READ_PTR(void);
void SET_PTR(ptr_t pointer, uint32_t byproduct);
uint32_t GET_PTR(ptr_t pointer);

#define GET_PTR8(pointer) cpu_readbyte(pointer.addr)
#define SET_PTR8(pointer, value) cpu_writebyte(pointer.addr, value)
#define GET_PTR16(pointer) cpu_readword(pointer.addr)
#define SET_PTR16(pointer, value) cpu_writeword(pointer.addr, value)
#define GET_PTR32(pointer) cpu_readdword(pointer.addr)
#define SET_PTR32(pointer, value) cpu_writedword(pointer.addr, value)
#define INCR_PTR(pointer, value) (SET_PTR(pointer, GET_PTR(pointer) + value))

#define SET_REGISTER8(reg, value) vm.regs[reg] = (uint32_t)((uint8_t)value)
#define SET_REGISTER16(reg, value) vm.regs[reg] = (uint32_t)((uint16_t)value)
#define SET_REGISTER32(reg, value) vm.regs[reg] = (uint32_t)value
#define GET_REGISTER8(reg) ((uint8_t)vm.regs[reg])
#define GET_REGISTER16(reg) ((uint16_t)vm.regs[reg])
#define GET_REGISTER32(reg) ((uint32_t)vm.regs[reg])
#define VALID_REGISTER(reg) (reg <= (MAX_REGS))

void run(uint32_t ramsize);

#endif
