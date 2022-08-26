#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "vm.h"

//        DEST - SRC
#define ADD_REGREG 0x00
#define ADD_REGPTR 0x01
#define ADD_REGDAT 0x02
#define ADD_PTRREG 0x03
#define ADD_PTRPTR 0x04
#define ADD_PTRDAT 0x05

void doadd(opcodepre_t prefix) {
    switch(prefix.mode) {
        case ADD_REGREG: {
            uint8_t first = READ_BYTE();
            uint8_t second = READ_BYTE();

            vm.regs[first] += vm.regs[second];

            // printf("add 0x%02x 0x%02x 0x%08x\n", first, second, vm.regs[first]);

            break;
        }
        case ADD_REGPTR: {
            uint8_t first = READ_BYTE();
            uint32_t second = GET_PTR(READ_PTR());

            vm.regs[first] += second;

            break;
        }
        case ADD_REGDAT: {
            uint8_t first = READ_BYTE();
            uint32_t second = READ_BYTE32();

            vm.regs[first] += second;

            // printf("add 0x%02x 0x%08x 0x%08x\n", first, second, vm.regs[first]);

            break;
        }
        case ADD_PTRPTR: {
            ptr_t first = READ_PTR();
            uint32_t second = GET_PTR(READ_PTR());
            INCR_PTR(first, second);
            break;
        }
        case ADD_PTRREG: {
            ptr_t first = READ_PTR();
            uint8_t second = READ_BYTE();

            INCR_PTR(first, GET_REGISTER32(second));
            break;
        }
        case ADD_PTRDAT: {
            ptr_t first = READ_PTR();
            uint32_t second = READ_BYTE32();

            INCR_PTR(first, second);
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

void dosub(opcodepre_t prefix) {
    switch(prefix.mode) {
        case SUB_REGREG: {
            uint8_t first = READ_BYTE();
            uint8_t second = READ_BYTE();

            vm.regs[first] -= vm.regs[second];

            // printf("sub 0x%02x 0x%02x 0x%08x\n", first, second, vm.regs[first]);
            break;
        }
        case SUB_REGPTR: {
            uint8_t first = READ_BYTE();
            uint32_t second = GET_PTR(READ_PTR());

            vm.regs[first] -= second;

            break;
        }
        case SUB_REGDAT: {
            uint8_t first = READ_BYTE();
            uint32_t second = READ_BYTE32();

            vm.regs[first] -= second;
            // printf("sub 0x%02x 0x%08x 0x%08x\n", first, second, vm.regs[first]);
            break;
        }
        case SUB_PTRPTR: {
            ptr_t first = READ_PTR();
            uint32_t second = GET_PTR(READ_PTR());
            INCR_PTR(first, -second);
            break;
        }
        case SUB_PTRREG: {
            ptr_t first = READ_PTR();
            uint8_t second = READ_BYTE();

            INCR_PTR(first, -GET_REGISTER32(second));
            break;
        }
        case SUB_PTRDAT: {
            ptr_t first = READ_PTR();
            uint32_t second = READ_BYTE32();

            INCR_PTR(first, -second);
            break;
        }

        default:
            printf("Instruction attempted to use a mode that doesn't exist! (code: 0x%02x)\n", prefix.mode);
            exit(1);
            return;
    }
}

//       DEST - SRC
#define IDIV_REGREG 0x00
#define IDIV_REGPTR 0x01
#define IDIV_REGDAT 0x02
#define IDIV_PTRPTR 0x03
#define IDIV_PTRREG 0x04
#define IDIV_PTRDAT 0x05

void doidiv(opcodepre_t prefix) {
    switch(prefix.mode) {
        case IDIV_REGREG: {
            uint8_t first = READ_BYTE();
            uint8_t second = READ_BYTE();

            vm.regs[REG_AX] = (int32_t)vm.regs[first] % (int32_t)vm.regs[second]; // tramples AX
            vm.regs[first] = (int32_t)vm.regs[first] / (int32_t)vm.regs[second];
            break;
        }
        case IDIV_REGPTR: {
            uint8_t first = READ_BYTE();
            uint32_t second = GET_PTR(READ_PTR());

            vm.regs[REG_AX] = (int32_t)vm.regs[first] % (int32_t)second; // tramples AX
            vm.regs[first] = (int32_t)vm.regs[first] / (int32_t)second;
            break;
        }
        case IDIV_REGDAT: {
            uint8_t first = READ_BYTE();
            uint32_t second = READ_BYTE32();

            vm.regs[REG_AX] = (int32_t)vm.regs[first] % (int32_t)second; // tramples AX
            vm.regs[first] = (int32_t)vm.regs[first] / (int32_t)second; 
            break;
        }
        case IDIV_PTRPTR: {
            ptr_t first = READ_PTR();
            uint32_t second = GET_PTR(READ_PTR());
            vm.regs[REG_AX] = (int32_t)GET_PTR(first) % (int32_t)second; // tramples AX
            SET_PTR(first, (int32_t)GET_PTR(first) / (int32_t)second);
            break;
        }
        case IDIV_PTRREG: {
            ptr_t first = READ_PTR();
            uint8_t second = READ_BYTE();
            vm.regs[REG_AX] = (int32_t)GET_PTR(first) % (int32_t)second; // tramples AX
            SET_PTR(first, (int32_t)GET_PTR(first) / (int32_t)GET_REGISTER32(second));
            break;
        }
        case IDIV_PTRDAT: {
            ptr_t first = READ_PTR();
            uint32_t second = READ_BYTE32();
            
            vm.regs[REG_AX] = (int32_t)GET_PTR(first) % (int32_t)second; // tramples AX
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

void dodiv(opcodepre_t prefix) {
    switch(prefix.mode) {
        case DIV_REGREG: {
            uint8_t first = READ_BYTE();
            uint8_t second = READ_BYTE();

            vm.regs[REG_AX] = vm.regs[first] % vm.regs[second]; // tramples AX
            vm.regs[first] = vm.regs[first] / vm.regs[second]; 
            break;
        }
        case DIV_REGPTR: {
            uint8_t first = READ_BYTE();
            uint32_t second = GET_PTR(READ_PTR());

            vm.regs[REG_AX] = vm.regs[first] % vm.regs[second]; // tramples AX
            vm.regs[first] = vm.regs[first] / second; 
            break;
        }
        case DIV_REGDAT: {
            uint8_t first = READ_BYTE();
            uint32_t second = READ_BYTE32();

            vm.regs[REG_AX] = vm.regs[first] % second; // tramples AX
            vm.regs[first] = vm.regs[first] / second; 
            break;
        }
        case DIV_PTRPTR: {
            ptr_t first = READ_PTR();
            uint32_t second = GET_PTR(READ_PTR());
            vm.regs[REG_AX] = GET_PTR(first) % second; // tramples AX
            SET_PTR(first, GET_PTR(first) / second);
            break;
        }
        case DIV_PTRREG: {
            ptr_t first = READ_PTR();
            uint8_t second = READ_BYTE();
            vm.regs[REG_AX] = GET_PTR(first) % vm.regs[second]; // tramples AX
            SET_PTR(first, GET_PTR(first) / GET_REGISTER32(second));
            break;
        }
        case DIV_PTRDAT: {
            ptr_t first = READ_PTR();
            uint32_t second = READ_BYTE32();
            
            vm.regs[REG_AX] = GET_PTR(first) % second; // tramples AX
            SET_PTR(first, GET_PTR(first) / second);
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

void domul(opcodepre_t prefix) {
    switch(prefix.mode) {
        case MUL_REGREG: {
            uint8_t first = READ_BYTE();
            uint8_t second = READ_BYTE();

            vm.regs[first] = vm.regs[first] * vm.regs[second];

            break;
        }
        case MUL_REGPTR: {
            uint8_t first = READ_BYTE();
            uint32_t second = GET_PTR(READ_PTR());

            vm.regs[first] = vm.regs[first] * second;

            break;
        }
        case MUL_REGDAT: {
            uint8_t first = READ_BYTE();
            uint32_t second = READ_BYTE32();

            vm.regs[first] = vm.regs[first] * second;

            break;
        }
        case MUL_PTRPTR: {
            ptr_t first = READ_PTR();
            uint32_t second = GET_PTR(READ_PTR()); 
            SET_PTR(first, GET_PTR(first) * second);
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

#define INC_REG 0x00
#define INC_PTR 0x01

void doinc(opcodepre_t prefix) {
    switch(prefix.mode) {
        case INC_REG: {
            uint8_t reg = READ_BYTE();
            vm.regs[reg]++;

            break;
        }
        case INC_PTR: {
            ptr_t pointer = READ_PTR(); 
            INCR_PTR(pointer, 1);
            break;
        }
        default:
            printf("Instruction attempted to use a mode that doesn't exist! (code: 0x%02x)\n", prefix.mode);
            exit(1);
            return;
    }
}

#define DEC_REG 0x00
#define DEC_PTR 0x01

void dodec(opcodepre_t prefix) {
    switch(prefix.mode) {
        case DEC_REG: {
            uint8_t reg = READ_BYTE();
            if(vm.regs[reg] > 0)
                vm.regs[reg]--;

            break;
        }
        case DEC_PTR: {
            ptr_t pointer = READ_PTR();
            INCR_PTR(pointer, -1); 
            break;
        }
        default:
            printf("Instruction attempted to use a mode that doesn't exist! (code: 0x%02x)\n", prefix.mode);
            exit(1);
            return;
    }
}

#define CMP_REGREG 0x00
#define CMP_REGPTR 0x01
#define CMP_REGDAT 0x02
#define CMP_PTRREG 0x03
#define CMP_PTRPTR 0x04
#define CMP_PTRDAT 0x05
#define CMP_DATREG 0x06
#define CMP_DATPTR 0x07
#define CMP_DATDAT 0x08

void docmp(opcodepre_t prefix) {
    uint32_t value1;
    uint32_t value2;
    switch(prefix.mode) {
        case CMP_REGREG: {
            uint8_t reg1 = READ_BYTE();
            uint8_t reg2 = READ_BYTE();
            value1 = GET_REGISTER32(reg1);
            value2 = GET_REGISTER32(reg2);
            break;
        }
        case CMP_REGPTR: {
            value1 = GET_REGISTER32(READ_BYTE());
            value2 = GET_PTR(READ_PTR());
            break;
        }
        case CMP_REGDAT: {
            value1 = GET_REGISTER32(READ_BYTE());
            value2 = READ_BYTE32();
            break;
        }
        case CMP_PTRREG: {
            value1 = GET_PTR(READ_PTR());
            value2 = GET_REGISTER32(READ_BYTE());
            break;
        }
        case CMP_PTRPTR: {
            value1 = GET_PTR(READ_PTR());
            value2 = GET_PTR(READ_PTR());
            break;
        }
        case CMP_PTRDAT: {
            value1 = GET_PTR(READ_PTR());
            value2 = READ_BYTE32();
            break;
        }
        case CMP_DATDAT: {
            value1 = READ_BYTE32();
            value2 = READ_BYTE32();
            break;
        }
        case CMP_DATPTR: {
            value1 = READ_BYTE32();
            value2 = GET_PTR(READ_PTR());
            break;
        }
        case CMP_DATREG: {
            value1 = READ_BYTE32();
            value2 = GET_REGISTER32(READ_BYTE());
            break;
        }
        default:
            printf("Instruction attempted to use a mode that doesn't exist! (code: 0x%02x)\n", prefix.mode);
            exit(1);
            return;
    }
    
    if(value1 == value2) {
        SET_FLAG(FLAG_ZF, 1);
    } else if(value1 > value2) {
        SET_FLAG(FLAG_ZF, 0);
        SET_FLAG(FLAG_CF, 1);
    } else if(value1 < value2) {
        SET_FLAG(FLAG_ZF, 0);
        SET_FLAG(FLAG_CF, 0);
    } else {
        printf("values (0x%08x, 0x%08x) meet no conditions! unexpected behaviour.\n", value1, value2);
        exit(1);
    }
}

#define SETEQ_REG 0x00
#define SETEQ_PTR 0x01

void doseteq(opcodepre_t prefix) {
    switch(prefix.mode) {
        case SETEQ_REG: {
            uint8_t reg = READ_BYTE();
            vm.regs[reg] = GET_FLAG(FLAG_ZF);
            break;
        }
        case SETEQ_PTR: {
            ptr_t pointer = READ_PTR();
            SET_PTR(pointer, GET_FLAG(FLAG_ZF));
            break;
        }
        default:
            printf("Instruction attempted to use a mode that doesn't exist! (code: 0x%02x)\n", prefix.mode);
            exit(1);
            return;
    }
}

#define SETNE_REG 0x00
#define SETNE_PTR 0x01

void dosetne(opcodepre_t prefix) {
    switch(prefix.mode) {
        case SETNE_REG: {
            uint8_t reg = READ_BYTE();
            vm.regs[reg] = !GET_FLAG(FLAG_ZF);
            break;
        }
        case SETNE_PTR: {
            ptr_t pointer = READ_PTR();
            SET_PTR(pointer, !GET_FLAG(FLAG_ZF));
            break;
        }
        default:
            printf("Instruction attempted to use a mode that doesn't exist! (code: 0x%02x)\n", prefix.mode);
            exit(1);
            return;
    }
}

#define SETLT_REG 0x00
#define SETLT_PTR 0x01

void dosetlt(opcodepre_t prefix) {
    switch(prefix.mode) {
        case SETLT_REG: {
            uint8_t reg = READ_BYTE();
            vm.regs[reg] = (!GET_FLAG(FLAG_ZF)) && (!GET_FLAG(FLAG_CF));
            break;
        }
        case SETLT_PTR: {
            ptr_t pointer = READ_PTR();
            SET_PTR(pointer, (!GET_FLAG(FLAG_ZF)) && (!GET_FLAG(FLAG_CF))); 
            break;
        }
        default:
            printf("Instruction attempted to use a mode that doesn't exist! (code: 0x%02x)\n", prefix.mode);
            exit(1);
            return;
    }
}

#define SETGT_REG 0x00
#define SETGT_PTR 0x01

void dosetgt(opcodepre_t prefix) {
    switch(prefix.mode) {
        case SETGT_REG: {
            uint8_t reg = READ_BYTE();
            vm.regs[reg] = (!GET_FLAG(FLAG_ZF)) && (GET_FLAG(FLAG_CF));
            break;
        }
        case SETGT_PTR: {
            ptr_t pointer = READ_PTR();
            SET_PTR(pointer, (!GET_FLAG(FLAG_ZF)) && (GET_FLAG(FLAG_CF))); 
            break;
        }
        default:
            printf("Instruction attempted to use a mode that doesn't exist! (code: 0x%02x)\n", prefix.mode);
            exit(1);
            return;
    }
}

#define SETLE_REG 0x00
#define SETLE_PTR 0x01

void dosetle(opcodepre_t prefix) {
    switch(prefix.mode) {
        case SETLE_REG: {
            uint8_t reg = READ_BYTE();
            vm.regs[reg] = (!GET_FLAG(FLAG_CF)) || GET_FLAG(FLAG_ZF);
            break;
        }
        case SETLE_PTR: {
            ptr_t pointer = READ_PTR();
            SET_PTR(pointer, (!GET_FLAG(FLAG_CF)) || GET_FLAG(FLAG_ZF));
            break;
        }
        default:
            printf("Instruction attempted to use a mode that doesn't exist! (code: 0x%02x)\n", prefix.mode);
            exit(1);
            return;
    }
}

#define SETGE_REG 0x00
#define SETGE_PTR 0x01

void dosetge(opcodepre_t prefix) {
    switch(prefix.mode) {
        case SETGE_REG: {
            uint8_t reg = READ_BYTE();
            vm.regs[reg] = (GET_FLAG(FLAG_CF)) || GET_FLAG(FLAG_ZF);
            break;
        }
        case SETGE_PTR: {
            ptr_t pointer = READ_PTR();
            SET_PTR(pointer, (GET_FLAG(FLAG_CF)) || GET_FLAG(FLAG_ZF)); 
            break;
        }
        default:
            printf("Instruction attempted to use a mode that doesn't exist! (code: 0x%02x)\n", prefix.mode);
            exit(1);
            return;
    }
}

//        DEST - SRC
#define AND_REGREG 0x00
#define AND_REGPTR 0x01
#define AND_REGDAT 0x02
#define AND_PTRPTR 0x03
#define AND_PTRREG 0x04
#define AND_PTRDAT 0x05

void doand(opcodepre_t prefix) {
    switch(prefix.mode) {
        case AND_REGREG: {
            uint8_t first = READ_BYTE();
            uint8_t second = READ_BYTE();

            vm.regs[first] &= vm.regs[second];
            break;
        }
        case AND_REGPTR: {
            uint8_t first = READ_BYTE();
            uint32_t second = GET_PTR(READ_PTR());

            vm.regs[first] &= second;
            break;
        }
        case AND_REGDAT: {
            uint8_t first = READ_BYTE();
            uint32_t second = READ_BYTE32();

            vm.regs[first] &= second;
            break;
        }
        case AND_PTRPTR: {
            ptr_t first = READ_PTR();
            uint32_t second = GET_PTR(READ_PTR());
            
            SET_PTR(first, GET_PTR(first) & second);
            break;
        }
        case AND_PTRREG: {
            ptr_t first = READ_PTR();
            uint8_t second = READ_BYTE();

            SET_PTR(first, GET_PTR(first) & GET_REGISTER32(second));
            break;
        }
        case AND_PTRDAT: {
            ptr_t first = READ_PTR();
            uint32_t second = READ_BYTE32();
 
            SET_PTR(first, GET_PTR(first) & second);
            break;
        }
        default:
            printf("Instruction attempted to use a mode that doesn't exist! (code: 0x%02x)\n", prefix.mode);
            exit(1);
            return;
    }
}

//        DEST - SRC
#define SHL_REGREG 0x00
#define SHL_REGPTR 0x01
#define SHL_REGDAT 0x02
#define SHL_PTRPTR 0x03
#define SHL_PTRREG 0x04
#define SHL_PTRDAT 0x05

void doshl(opcodepre_t prefix) {
    switch(prefix.mode) {
        case SHL_REGREG: {
            uint8_t first = READ_BYTE();
            uint8_t second = READ_BYTE();

            vm.regs[first] <<= vm.regs[second];
            break;
        }
        case SHL_REGPTR: {
            uint8_t first = READ_BYTE();
            uint32_t second = GET_PTR(READ_PTR());

            vm.regs[first] <<= second;
            break;
        }
        case SHL_REGDAT: {
            uint8_t first = READ_BYTE();
            uint32_t second = READ_BYTE32();

            vm.regs[first] <<= second;
            break;
        }
        case SHL_PTRPTR: {
            ptr_t first = READ_PTR();
            uint32_t second = GET_PTR(READ_PTR());

            SET_PTR(first, GET_PTR(first) << second);
            break;
        }
        case SHL_PTRREG: {
            ptr_t first = READ_PTR();
            uint8_t second = READ_BYTE();

            SET_PTR(first, GET_PTR(first) << GET_REGISTER32(second));        
            break;
        }
        case SHL_PTRDAT: {
            ptr_t first = READ_PTR();
            uint32_t second = READ_BYTE32();
 
            SET_PTR(first, GET_PTR(first) << second);
            break;
        }
        default:
            printf("Instruction attempted to use a mode that doesn't exist! (code: 0x%02x)\n", prefix.mode);
            exit(1);
            return;
    }
}

//        DEST - SRC
#define SHR_REGREG 0x00
#define SHR_REGPTR 0x01
#define SHR_REGDAT 0x02
#define SHR_PTRPTR 0x03
#define SHR_PTRREG 0x04
#define SHR_PTRDAT 0x05

void doshr(opcodepre_t prefix) {
    switch(prefix.mode) {
        case SHR_REGREG: {
            uint8_t first = READ_BYTE();
            uint8_t second = READ_BYTE();

            vm.regs[first] >>= vm.regs[second];
            break;
        }
        case SHR_REGPTR: {
            uint8_t first = READ_BYTE();
            uint32_t second = GET_PTR(READ_PTR());

            vm.regs[first] >>= second;
            break;
        }
        case SHR_REGDAT: {
            uint8_t first = READ_BYTE();
            uint32_t second = READ_BYTE32();

            vm.regs[first] >>= second;
            break;
        }
        case SHR_PTRPTR: {
            ptr_t first = READ_PTR();
            uint32_t second = GET_PTR(READ_PTR());
            
            SET_PTR(first, GET_PTR(first) >> second);
            break;
        }
        case SHR_PTRREG: {
            ptr_t first = READ_PTR();
            uint8_t second = READ_BYTE();
 
            SET_PTR(first, GET_PTR(first) >> GET_REGISTER32(second));
            break;
        }
        case SHR_PTRDAT: {
            ptr_t first = READ_PTR();
            uint32_t second = READ_BYTE32();
 
            SET_PTR(first, GET_PTR(first) >> second);
            break;
        }
        default:
            printf("Instruction attempted to use a mode that doesn't exist! (code: 0x%02x)\n", prefix.mode);
            exit(1);
            return;
    }
}

//        DEST - SRC
#define XOR_REGREG 0x00
#define XOR_REGPTR 0x01
#define XOR_REGDAT 0x02
#define XOR_PTRPTR 0x03
#define XOR_PTRREG 0x04
#define XOR_PTRDAT 0x05

void doxor(opcodepre_t prefix) {
    switch(prefix.mode) {
        case XOR_REGREG: {
            uint8_t first = READ_BYTE();
            uint8_t second = READ_BYTE();

            vm.regs[first] ^= vm.regs[second];
            break;
        }
        case XOR_REGPTR: {
            uint8_t first = READ_BYTE();
            uint32_t second = GET_PTR(READ_PTR());

            vm.regs[first] ^= second;
            break;
        }
        case XOR_REGDAT: {
            uint8_t first = READ_BYTE();
            uint32_t second = READ_BYTE32();

            vm.regs[first] ^= second;
            break;
        }
        case XOR_PTRPTR: {
            ptr_t first = READ_PTR();
            uint32_t second = GET_PTR(READ_PTR());
            
            SET_PTR(first, GET_PTR(first) ^ second);
            break;
        }
        case XOR_PTRREG: {
            ptr_t first = READ_PTR();
            uint8_t second = READ_BYTE();

            SET_PTR(first, GET_PTR(first) ^ GET_REGISTER32(second));
            break;
        }
        case XOR_PTRDAT: {
            ptr_t first = READ_PTR();
            uint32_t second = READ_BYTE32();

            SET_PTR(first, GET_PTR(first) ^ second);
            break;
        }
        default:
            printf("Instruction attempted to use a mode that doesn't exist! (code: 0x%02x)\n", prefix.mode);
            exit(1);
            return;
    }
}

//        DEST - SRC
#define OR_REGREG 0x00
#define OR_REGPTR 0x01
#define OR_REGDAT 0x02
#define OR_PTRPTR 0x03
#define OR_PTRREG 0x04
#define OR_PTRDAT 0x05

void door(opcodepre_t prefix) {
    switch(prefix.mode) {
        case OR_REGREG: {
            uint8_t first = READ_BYTE();
            uint8_t second = READ_BYTE();

            vm.regs[first] |= vm.regs[second];
            break;
        }
        case OR_REGPTR: {
            uint8_t first = READ_BYTE();
            uint32_t second = GET_PTR(READ_PTR());

            vm.regs[first] |= second;
            break;
        }
        case OR_REGDAT: {
            uint8_t first = READ_BYTE();
            uint32_t second = READ_BYTE32();

            vm.regs[first] |= second;
            break;
        }
        case OR_PTRPTR: {
            ptr_t first = READ_PTR();
            uint32_t second = GET_PTR(READ_PTR());
            
            SET_PTR(first, GET_PTR(first) | second);
            break;
        }
        case OR_PTRREG: {
            ptr_t first = READ_PTR();
            uint8_t second = READ_BYTE();
 
            SET_PTR(first, GET_PTR(first) | GET_REGISTER32(second));
            break;
        }
        case OR_PTRDAT: {
            ptr_t first = READ_PTR();
            uint32_t second = READ_BYTE32();

            SET_PTR(first, GET_PTR(first) | second);
            break;
        }
        default:
            printf("Instruction attempted to use a mode that doesn't exist! (code: 0x%02x)\n", prefix.mode);
            exit(1);
            return;
    }
}

#define NOT_REG 0x00
#define NOT_PTR 0x01

void donot(opcodepre_t prefix) {
    switch(prefix.mode) {
        case NOT_REG: {
            uint8_t reg = READ_BYTE();
            vm.regs[reg] = ~vm.regs[reg];
            break;
        }
        case NOT_PTR: {
            ptr_t pointer = READ_PTR();
            
            SET_PTR(pointer, ~GET_PTR(pointer));
            break;
        }
        default:
            printf("Instruction attempted to use a mode that doesn't exist! (code: 0x%02x)\n", prefix.mode);
            exit(1);
            return;
    }
}

#define NEG_REG 0x00
#define NEG_PTR 0x01

void doneg(opcodepre_t prefix) {
    switch(prefix.mode) {
        case NEG_REG: {
            uint8_t reg = READ_BYTE();
            vm.regs[reg] = -vm.regs[reg];
            break;
        }
        case NEG_PTR: {
            ptr_t pointer = READ_PTR();
            SET_PTR(pointer, -GET_PTR(pointer));
            break;
        }
        default:
            printf("Instruction attempted to use a mode that doesn't exist! (code: 0x%02x)\n", prefix.mode);
            exit(1);
            return;
    }
}

//        DEST - SRC
#define TEST_REGREG 0x00
#define TEST_REGPTR 0x01
#define TEST_REGDAT 0x02
#define TEST_PTRPTR 0x03
#define TEST_PTRREG 0x04
#define TEST_PTRDAT 0x05

void dotest(opcodepre_t prefix) {
    switch(prefix.mode) {
        case TEST_REGREG: {
            uint8_t first = READ_BYTE();
            uint8_t second = READ_BYTE();
            uint32_t temp = vm.regs[first] & vm.regs[second]; 

            if(temp == 0) {
                SET_FLAG(FLAG_ZF, 1);
            } else if(temp < 0) {
                SET_FLAG(FLAG_SF, 1);
                SET_FLAG(FLAG_ZF, 0);
            } else if(temp > 0) {
                SET_FLAG(FLAG_SF, 0);
                SET_FLAG(FLAG_ZF, 0);
            }
            break;
        }
        case TEST_REGPTR: {
            uint8_t first = READ_BYTE();
            uint32_t second = GET_PTR(READ_PTR());
            uint32_t temp = vm.regs[first] & second;

            if(temp == 0) {
                SET_FLAG(FLAG_ZF, 1);
            } else if(temp < 0) {
                SET_FLAG(FLAG_SF, 1);
                SET_FLAG(FLAG_ZF, 0);
            } else if(temp > 0) {
                SET_FLAG(FLAG_SF, 0);
                SET_FLAG(FLAG_ZF, 0);
            }
            break;
        }
        case TEST_REGDAT: {
            uint32_t first = vm.regs[READ_BYTE()];
            uint32_t second = READ_BYTE32();
            uint32_t temp = first & second; 

            SET_FLAG(FLAG_OF, 0);
            SET_FLAG(FLAG_CF, 0);
            if(temp == 0) {
                SET_FLAG(FLAG_ZF, 1);
            } else if(temp < 0) {
                SET_FLAG(FLAG_SF, 1);
                SET_FLAG(FLAG_ZF, 0);
            } else if(temp > 0) {
                SET_FLAG(FLAG_SF, 0);
                SET_FLAG(FLAG_ZF, 0);
            }
            break;
        }
        case TEST_PTRPTR: {
            ptr_t first = READ_PTR();
            uint32_t second = GET_PTR(READ_PTR());
            uint32_t temp = GET_PTR(first) & second;

            if(temp == 0) {
                SET_FLAG(FLAG_ZF, 1);
            } else if(temp < 0) {
                SET_FLAG(FLAG_SF, 1);
                SET_FLAG(FLAG_ZF, 0);
            } else if(temp > 0) {
                SET_FLAG(FLAG_SF, 0);
                SET_FLAG(FLAG_ZF, 0);
            }
            break;
        }
        case TEST_PTRREG: {
            ptr_t first = READ_PTR();
            uint8_t second = READ_BYTE();
            uint32_t temp = GET_PTR(first) & second;

            if(temp == 0) {
                SET_FLAG(FLAG_ZF, 1);
            } else if(temp < 0) {
                SET_FLAG(FLAG_SF, 1);
                SET_FLAG(FLAG_ZF, 0);
            } else if(temp > 0) {
                SET_FLAG(FLAG_SF, 0);
                SET_FLAG(FLAG_ZF, 0);
            }
            break;
        }
        case TEST_PTRDAT: {
            ptr_t first = READ_PTR();
            uint32_t second = READ_BYTE32();
            uint32_t temp = GET_PTR(first) & second;


            if(temp == 0) {
                SET_FLAG(FLAG_ZF, 1);
            } else if(temp < 0) {
                SET_FLAG(FLAG_SF, 1);
                SET_FLAG(FLAG_ZF, 0);
            } else if(temp > 0) {
                SET_FLAG(FLAG_SF, 0);
                SET_FLAG(FLAG_ZF, 0);
            }
            break;
        }
        default:
            printf("Instruction attempted to use a mode that doesn't exist! (code: 0x%02x)\n", prefix.mode);
            exit(1);
            return;
    }
}
