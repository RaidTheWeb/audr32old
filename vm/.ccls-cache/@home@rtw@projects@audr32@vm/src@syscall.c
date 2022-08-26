#include "vm.h"


#define MAX_SYSCALLS 64

typedef struct {
    uint16_t set;
    uint32_t addr; // handle address
} syscalltableent_t;

struct SyscallTable {
    syscalltableent_t sysent[MAX_SYSCALLS + 1];
};

struct SyscallTable systable;

static void syscall(uint32_t num) {
    if(systable.sysent[num].set) {
        vm.regs[REG_SP] -= 4;
        cpu_writedword(vm.regs[REG_SP], vm.regs[REG_SP]);
        vm.regs[REG_IP] = systable.sysent[num].addr;

    } else {
        void audr32_safeexception(uint32_t exc);
        audr32_safeexception(EXC_BADSYS);
    }
}

void dosyscall(opcodepre_t prefix) {
    // ax: syscall number
    // bx: arg0
    // cx: arg1
    // dx: arg2
    // di: arg3
    // si: arg4
    
    uint32_t num = vm.regs[REG_AX];
    // actual args are handled by the syscall handler code
    syscall(num);
}

static uint32_t syscall_read(uint16_t port) {
    return 0;
}

static void syscall_write(uint16_t port, uint32_t data) {
    uint8_t entries = cpu_readbyte(data++);
    for(size_t i = 0; i < entries; i += 5) {
        uint8_t num = cpu_readbyte(data + i);
        uint32_t addr = cpu_readdword(data + i + 1);
        syscalltableent_t systableentry = {
            .set = 1,
            .addr = addr
        };
        systable.sysent[num] = systableentry;
    }
}

void syscall_init(void) {
    vm.ports[0x8A].set = 1;
    vm.ports[0x8A].read = syscall_read;
    vm.ports[0x8A].write = syscall_write;
}
