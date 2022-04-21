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

            vm.regs[first] = vm.regs[first] / vm.regs[second];
            break;
        }
        case DIV_REGPTR: {
            uint8_t first = READ_BYTE();
            uint32_t second = GET_PTR(READ_PTR());

            vm.regs[first] = vm.regs[first] / second;;
            break;
        }
        case DIV_REGDAT: {
            uint8_t first = READ_BYTE();
            uint32_t second = READ_BYTE32();

            vm.regs[first] = vm.regs[first] / second;
            break;
        }
        case DIV_PTRPTR: {
            ptr_t first = READ_PTR();
            uint32_t second = GET_PTR(READ_PTR()); 
            SET_PTR(first, GET_PTR(first) / second);
            break;
        }
        case DIV_PTRREG: {
            ptr_t first = READ_PTR();
            uint8_t second = READ_BYTE();
            SET_PTR(first, GET_PTR(first) / GET_REGISTER32(second));
            break;
        }
        case DIV_PTRDAT: {
            ptr_t first = READ_PTR();
            uint32_t second = READ_BYTE32();

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

            vm.regs[first] = vm.regs[first] * second;;
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

/*//        DEST - SR
#define CMP_REGREG 0x00
#define CMP_REGDAT 0x01
#define CMP_DATDAT 0x02
#define CMP_DATREG 0x03

#define CMP_OP_EQ 0x00
#define CMP_OP_LT 0x01
#define CMP_OP_GT 0x02
#define CMP_OP_LE 0x03
#define CMP_OP_GE 0x04

void docmp(opcodepre_t prefix) {
    switch(prefix.mode) {
        case CMP_REGREG: {
            uint8_t cmpmode = READ_BYTE();
            uint8_t first = READ_BYTE();
            uint8_t second = READ_BYTE();
            switch(cmpmode) {
                case CMP_OP_EQ: {
                    vm.regs[REG_AX] = GET_REGISTER32(first) == GET_REGISTER32(second);
                    break;
                }
                case CMP_OP_LT: {
                    vm.regs[REG_AX] = GET_REGISTER32(first) < GET_REGISTER32(second);
                    break;
                }
                case CMP_OP_GT: {
                    vm.regs[REG_AX] = GET_REGISTER32(first) > GET_REGISTER32(second);
                    break;
                }
                case CMP_OP_LE: {
                    vm.regs[REG_AX] = GET_REGISTER32(first) <= GET_REGISTER32(second);
                    break;
                }
                case CMP_OP_GE: {
                    vm.regs[REG_AX] = GET_REGISTER32(first) >= GET_REGISTER32(second);
                    break;
                }
                default:
                    printf("Instruction attempted to use a CMP operation mode that doesn't exist! (code: 0x%02x)\n", cmpmode);
                    exit(1);
                    return;
            }
            break;
        }
        case CMP_REGDAT: {
            uint8_t cmpmode = READ_BYTE();
            uint8_t first = READ_BYTE();
            uint32_t second = READ_BYTE32();
            switch(cmpmode) {
                case CMP_OP_EQ: {
                    vm.regs[REG_AX] = GET_REGISTER32(first) == second;
                    break;
                }
                case CMP_OP_LT: {
                    vm.regs[REG_AX] = GET_REGISTER32(first) < second;
                    break;
                }
                case CMP_OP_GT: {
                    vm.regs[REG_AX] = GET_REGISTER32(first) > second;
                    break;
                }
                case CMP_OP_LE: {
                    vm.regs[REG_AX] = GET_REGISTER32(first) <= second;
                    break;
                }
                case CMP_OP_GE: {
                    vm.regs[REG_AX] = GET_REGISTER32(first) >= second;
                    break;
                }
                default:
                    printf("Instruction attempted to use a CMP operation mode that doesn't exist! (code: 0x%02x)\n", cmpmode);
                    exit(1);
                    return;
            }
            break;
        }
        case CMP_DATDAT: {
            uint8_t cmpmode = READ_BYTE();
            uint32_t first = READ_BYTE32();
            uint32_t second = READ_BYTE32();
            switch(cmpmode) {
                case CMP_OP_EQ: {
                    vm.regs[REG_AX] = first == second;
                    break;
                }
                case CMP_OP_LT: {
                    vm.regs[REG_AX] = first < second;
                    break;
                }
                case CMP_OP_GT: {
                    vm.regs[REG_AX] = first > second;
                    break;
                }
                case CMP_OP_LE: {
                    vm.regs[REG_AX] = first <= second;
                    break;
                }
                case CMP_OP_GE: {
                    vm.regs[REG_AX] = first >= second;
                    break;
                }
                default:
                    printf("Instruction attempted to use a CMP operation mode that doesn't exist! (code: 0x%02x)\n", cmpmode);
                    exit(1);
                    return;
            }
            break;
        }
        case CMP_DATREG: {
            uint8_t cmpmode = READ_BYTE();
            uint32_t first = READ_BYTE32();
            uint8_t second = READ_BYTE();
            switch(cmpmode) {
                case CMP_OP_EQ: {
                    vm.regs[REG_AX] = first == GET_REGISTER32(second);
                    break;
                }
                case CMP_OP_LT: {
                    vm.regs[REG_AX] = first < GET_REGISTER32(second);
                    break;
                }
                case CMP_OP_GT: {
                    vm.regs[REG_AX] = first > GET_REGISTER32(second);
                    break;
                }
                case CMP_OP_LE: {
                    vm.regs[REG_AX] = first <= GET_REGISTER32(second);
                    break;
                }
                case CMP_OP_GE: {
                    vm.regs[REG_AX] = first >= GET_REGISTER32(second);
                    break;
                }
                default:
                    printf("Instruction attempted to use a CMP operation mode that doesn't exist! (code: 0x%02x)\n", cmpmode);
                    exit(1);
                    return;
            }
            break;
        }
        default:
            printf("Instruction attempted to use a mode that doesn't exist! (code: 0x%02x)\n", prefix.mode);
            exit(1);
            return;
    }
}*/

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
            value1 = GET_REGISTER32(READ_BYTE());
            value2 = GET_REGISTER32(READ_BYTE());
            break;
        }
        case CMP_REGPTR: {
            value1 = GET_REGISTER32(READ_BYTE());
            value2 = GET_PTR(READ_PTR());
//            if(check != 0)
//                vm.regs[REG_IP] = GET_REGISTER32(location);
            break;
        }
        case CMP_REGDAT: {
            value1 = GET_REGISTER32(READ_BYTE());
            value2 = READ_BYTE32();
//            if(check != 0)
//                vm.regs[REG_IP] = GET_REGISTER32(location);
            break;
        }
        case CMP_PTRREG: {
            value1 = GET_PTR(READ_PTR());
            value2 = GET_REGISTER32(READ_BYTE());
//            if(GET_REGISTER32(check) != 0)
//                vm.regs[REG_IP] = location;
            break;
        }
        case CMP_PTRPTR: {
            value1 = GET_PTR(READ_PTR());
            value2 = GET_PTR(READ_PTR());

//            if(check != 0)
//                vm.regs[REG_IP] = location;
            break;
        }
        case CMP_PTRDAT: {
            value1 = GET_PTR(READ_PTR());
            value2 = READ_BYTE32();

//            if(check != 0)
//                vm.regs[REG_IP] = location;
            break;
        }
        case CMP_DATDAT: {
            value1 = READ_BYTE32();
            value2 = READ_BYTE32();

//            if(check != 0)
//                vm.regs[REG_IP] = location;
            break;
        }
        case CMP_DATPTR: {
            value1 = READ_BYTE32();
            value2 = GET_PTR(READ_PTR());

//            if(check != 0)
//                vm.regs[REG_IP] = location;
            break;
        }
        case CMP_DATREG: {
            value1 = READ_BYTE32();
            value2 = GET_REGISTER32(READ_BYTE());

//            if(GET_REGISTER32(check) != 0)
//                vm.regs[REG_IP] = location;
            break;
        }
        default:
            printf("Instruction attempted to use a mode that doesn't exist! (code: 0x%02x)\n", prefix.mode);
            exit(1);
            return;
    }
    
    if(value1 == value2) {
        vm.flags[FLAG_ZF] = 1;
        vm.flags[FLAG_CF] = 0;
    } else if(value1 > value2) {
        vm.flags[FLAG_ZF] = 0;
        vm.flags[FLAG_CF] = 0;
    } else if(value1 < value2) {
        vm.flags[FLAG_ZF] = 0;
        vm.flags[FLAG_CF] = 1;
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
            
            SET_PTR(pointer, GET_PTR(pointer) ^ -1);
            break;
        }
        default:
            printf("Instruction attempted to use a mode that doesn't exist! (code: 0x%02x)\n", prefix.mode);
            exit(1);
            return;
    }
}
