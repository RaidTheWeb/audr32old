#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdint.h>

#include "common.h"
#include "compiler.h"
#include "datamap.h"
#include "emitter.h"
#include "lexer.h"



/*********************************************************************
 * Audr32 Assembler
 * compiler.c - Compile down to flat binary.
 *
 * Do everything needed in here. >//<
 * Assembler specification is in `../ASM.txt`
 *********************************************************************/


compilerjob_t job;
struct Emitter emitter;
labelmap_t *labels; 

struct Relocatable {
    char *symbol;
    uint32_t loc; // location where to relocate
    uint8_t mode; // 0x01, 0x02, 0x03 (8 bit, 16 bit, and 32 bit respectively)
};

struct Relocatable *relocatables[INITIAL_CAPACITY];
int relocatablepointer = 0;

struct Parser {
    struct Token curtoken;
    struct Token peektoken;
//    struct Lexer *lexer;
};
//struct Parser comparser;

#define DEF_REG(_n, _dat) \
    else if(strcmp(text, #_n) == 0) { \
        return _dat; \
    }

static uint8_t resolveregister(char *text) {
    if(strcmp(text, "ax") == 0) {
        return 0x01;
    }
    DEF_REG(bx, 0x02)
    DEF_REG(cx, 0x03)
    DEF_REG(dx, 0x04)
    DEF_REG(si, 0x05)
    DEF_REG(di, 0x06)
    DEF_REG(sp, 0x07)
    DEF_REG(bp, 0x08)
    DEF_REG(ip, 0x09)
    DEF_REG(r0, 0x0A)
    DEF_REG(r1, 0x0B)
    DEF_REG(r2, 0x0C)
    DEF_REG(r3, 0x0D)
    DEF_REG(r4, 0x0E)
    DEF_REG(r5, 0x0F)
    DEF_REG(r6, 0x10)
    DEF_REG(r7, 0x11)
    DEF_REG(r8, 0x12)
    DEF_REG(r9, 0x13)
    DEF_REG(r10, 0x14)
    DEF_REG(r11, 0x15)
    DEF_REG(r12, 0x16)
    DEF_REG(r13, 0x17)
    DEF_REG(r14, 0x18)
    DEF_REG(r15, 0x19)

    return 0x00; // NULL (Should not be reached)
}

static char *resolvetoken(int type) {
    switch(type) {
        case TOK_ADD:               return "ADD";
        case TOK_AND:               return "AND";
        case TOK_ASTERISK:          return "ASTERISK";
        case TOK_BAND:              return "BAND";
        case TOK_BOR:               return "BOR";
        case TOK_CALL:              return "CALL";
        case TOK_CHAR:              return "CHAR";
        case TOK_CMP:               return "CMP";
        case TOK_COLON:             return "COLON";
        case TOK_COMMA:             return "COMMA";
        case TOK_DIV:               return "DIV";
        case TOK_EOF:               return "EOF";
        case TOK_HLT:               return "HLT";
        case TOK_HASHTAG:           return "HASHTAG";
        case TOK_IDIV:              return "IDIV";
        case TOK_INT:               return "INT";
        case TOK_INX:               return "INX";
        case TOK_JMP:               return "JMP";
        case TOK_JNZ:               return "JNZ";
        case TOK_JZ:                return "JZ";
        case TOK_LABEL:             return "LABEL";
        case TOK_LBRACKET:          return "LBRACKET";
        case TOK_LSHIFT:            return "LSHIFT";
        case TOK_LSQUARE:           return "LSQUARE";
        case TOK_MINUS:             return "MINUS";
        case TOK_MOV:               return "MOV";
        case TOK_MUL:               return "MUL";
        case TOK_NEWLINE:           return "NEWLINE";
        case TOK_NOP:               return "NOP";
        case TOK_NOT:               return "NOT";
        case TOK_NUMBER:            return "NUMBER";
        case TOK_OR:                return "OR";
        case TOK_OUTX:              return "OUTX";
        case TOK_PERIOD:            return "PERIOD";
        case TOK_PLUS:              return "PLUS";
        case TOK_POP:               return "POP";
        case TOK_PUSH:              return "PUSH";
        case TOK_REGISTER:          return "REGISTER";
        case TOK_RBRACKET:          return "RBRACKET";
        case TOK_RET:               return "RET";
        case TOK_RSHIFT:            return "RSHIFT";
        case TOK_RSQUARE:           return "RSQUARE";
        case TOK_SHL:               return "SHL";
        case TOK_SHR:               return "SHR";
        case TOK_SLASH:             return "SLASH";
        case TOK_STRING:            return "STRING";
        case TOK_SUB:               return "SUB";
        case TOK_XOR:               return "XOR";
        case TOK_SETEQ:             return "SETEQ";
        case TOK_SETNE:             return "SETNE";
        case TOK_SETLT:             return "SETLT";
        case TOK_SETGT:             return "SETGT";
        case TOK_SETLE:             return "SETLE";
        case TOK_SETGE:             return "SETGE";
        case TOK_LEA:               return "LEA";
        case TOK_NEG:               return "NEG";
        case TOK_TEST:              return "TEST";
        case TOK_SYSCALL:           return "SYSCALL";
    }
    return "NOP"; // Default to NOP
}

static int checktoken(struct Parser *parser, int type) {
    return type == parser->curtoken.type;
}

static int checkpeek(struct Parser *parser, int type) {
    return type == parser->peektoken.type;
}

static void parserabort(char *message) {
    printf("%s\n", message);
    exit(1);
}

static void nexttoken(struct Parser *parser, struct Lexer *lexer) {
    parser->curtoken = parser->peektoken;
    parser->peektoken = gettoken(lexer);
}

static void match(struct Parser *parser, struct Lexer *lexer, int type) {
    if(!checktoken(parser, type)) {
        char buf[128];
        sprintf(buf, "Expected %s got %s on line %lu", resolvetoken(type), resolvetoken(parser->curtoken.type), lexer->line);
        parserabort(buf);
    }
    nexttoken(parser, lexer);
}

static void checknewline(struct Parser *parser, struct Lexer *lexer) {
    match(parser, lexer, TOK_NEWLINE);
    while(checktoken(parser, TOK_NEWLINE))
        nexttoken(parser, lexer);
}

static void parsesymbol(struct Parser *parser, struct Lexer *lexer) {
    struct Token labeltok = parser->curtoken;
    // printf("reference to %s\n", labeltok.text);
    nexttoken(parser, lexer); 
    labelmapent_t *label = labelmapget(labels, labeltok.text);
    if(!(label == NULL)) {
        emitbyte32(&emitter, label->value); // emit label
        // printf("predefined label location: 0x%08x\n", label->value);
    } else {
        // printf("reference to a label that doesn't exist.\n");
        uint32_t current = emitter.written;
        emitbyte32(&emitter, 0x00000000); // emit zeros (replace later)
        struct Relocatable *relocatable = (struct Relocatable *)malloc(sizeof(struct Relocatable *)); // 32 bit relocatable located at this address.
        relocatable->symbol = strdup(labeltok.text);
        relocatable->loc = current;
        relocatable->mode = 0x03;
        relocatables[relocatablepointer++] = relocatable;
    }
}

static void parseptr(struct Parser *parser, struct Lexer *lexer) {
    nexttoken(parser, lexer);
    if(checktoken(parser, TOK_NUMBER)) {
        int mode = 0;
        switch(strtol(parser->curtoken.text, NULL, 10)) {
            case 8:
                mode = 0x01;
                break;
            case 16:
                mode = 0x02;
                break;
            case 32:
                mode = 0x03;
                break;
            default:
                parserabort("Unexpected pointer mode.");
                break;
        }
        match(parser, lexer, TOK_NUMBER);
        // printf("reading pointer location");
        match(parser, lexer, TOK_COLON);
        if(checktoken(parser, TOK_NUMBER)) {
            emitbyte(&emitter, mode);
            emitbyte32(&emitter, strtol(parser->curtoken.text, NULL, 10));
            match(parser, lexer, TOK_NUMBER);
        } else if(checktoken(parser, TOK_REGISTER)) {
            emitbyte(&emitter, 0x04 + (mode - 1));
            emitbyte(&emitter, resolveregister(parser->curtoken.text));
            match(parser, lexer, TOK_REGISTER);
        } else if(checktoken(parser, TOK_LABEL)) {
            emitbyte(&emitter, mode);
            parsesymbol(parser, lexer);
        }
    } else if(checktoken(parser, TOK_REGISTER)) {
        emitbyte(&emitter, 0x06); // REG32
        emitbyte(&emitter, resolveregister(parser->curtoken.text));
        match(parser, lexer, TOK_REGISTER);
    } else if(checktoken(parser, TOK_LABEL)) {
        emitbyte(&emitter, 0x03); // uh ok
        parsesymbol(parser, lexer);
    }
    if(checktoken(parser, TOK_COLON)) { // offset
        match(parser, lexer, TOK_COLON);
        int isneg = 0; // positive
        if(checktoken(parser, TOK_MINUS)) {
            isneg = 1; // negative
            match(parser, lexer, TOK_MINUS);
        }
        if(checktoken(parser, TOK_NUMBER)) {
            if(!isneg) emitbyte32(&emitter, strtol(parser->curtoken.text, NULL, 10)); 
            else emitbyte32(&emitter, -strtol(parser->curtoken.text, NULL, 10));
            match(parser, lexer, TOK_NUMBER);
        }
    } else {
        emitbyte32(&emitter, 0x00000000); // null offset
    }
    match(parser, lexer, TOK_RSQUARE);
}

static void parsemov(struct Parser *parser, struct Lexer *lexer) {
    emitbyte(&emitter, 0x02); // MOV
    if(checktoken(parser, TOK_REGISTER)) {
        uint8_t reg = resolveregister(parser->curtoken.text);
        nexttoken(parser, lexer);
        match(parser, lexer, TOK_COMMA);
        if(checktoken(parser, TOK_NUMBER)) {
            emitbyte(&emitter, 0x02); // REGDAT
            emitbyte(&emitter, reg);
            emitbyte32(&emitter, strtol(parser->curtoken.text, NULL, 10));
            match(parser, lexer, TOK_NUMBER);
        } else if(checktoken(parser, TOK_CHAR)) {
            emitbyte(&emitter, 0x02); // REGDAT
            emitbyte(&emitter, reg);
            emitbyte32(&emitter, (char)parser->curtoken.text[0]);
            match(parser, lexer, TOK_CHAR);
        } else if(checktoken(parser, TOK_REGISTER)) {
            emitbyte(&emitter, 0x00); // REGREG
            emitbyte(&emitter, reg);
            emitbyte(&emitter, resolveregister(parser->curtoken.text));
            match(parser, lexer, TOK_REGISTER);
        } else if(checktoken(parser, TOK_LSQUARE)) { // possibly a pointer
            emitbyte(&emitter, 0x01); // REGPTR
            emitbyte(&emitter, reg);
            parseptr(parser, lexer);
        } else if(checktoken(parser, TOK_LABEL)) {
            emitbyte(&emitter, 0x02); // REGDAT
            emitbyte(&emitter, reg);
            parsesymbol(parser, lexer);
        }
    } else if(checktoken(parser, TOK_LSQUARE)) {
        uint32_t loc = emitter.written; // used when replacing the dummy mode with the real mode later
        emitbyte(&emitter, 0x00); // dummy mode (replaced later)
        parseptr(parser, lexer);
        match(parser, lexer, TOK_COMMA);
        if(checktoken(parser, TOK_REGISTER)) {
            relocatebyte(&emitter, loc, 0x03); // PTRREG
            uint8_t reg = resolveregister(parser->curtoken.text);
            emitbyte(&emitter, reg);
            match(parser, lexer, TOK_REGISTER);
        } else if(checktoken(parser, TOK_NUMBER)) {
            relocatebyte(&emitter, loc, 0x04); // PTRDAT
            emitbyte32(&emitter, strtol(parser->curtoken.text, NULL, 10));
            match(parser, lexer, TOK_NUMBER);
        } else if(checktoken(parser, TOK_CHAR)) {
            relocatebyte(&emitter, loc, 0x04); // PTRDAT
            emitbyte32(&emitter, (char)parser->curtoken.text[0]);
            match(parser, lexer, TOK_CHAR);
        } else if(checktoken(parser, TOK_LABEL)) {
            relocatebyte(&emitter, loc, 0x04); // PTRDAT
            parsesymbol(parser, lexer);
        } else if(checktoken(parser, TOK_LSQUARE)) {
            relocatebyte(&emitter, loc, 0x05); // PTRPTR
            parseptr(parser, lexer);
        }
    }
}

static void parsejmp(struct Parser *parser, struct Lexer *lexer) {
    uint32_t start = emitter.written;
    emitbyte(&emitter, 0x04); // JMP
    if(checktoken(parser, TOK_REGISTER)) {
        emitbyte(&emitter, 0x00); // REG
        emitbyte(&emitter, resolveregister(parser->curtoken.text));
        match(parser, lexer, TOK_REGISTER);
    } else if(checktoken(parser, TOK_LSQUARE)) {
        emitbyte(&emitter, 0x01); // PTR
        parseptr(parser, lexer);
    } else if(checktoken(parser, TOK_NUMBER)) {
        emitbyte(&emitter, 0x02); // DAT
        emitbyte32(&emitter, strtol(parser->curtoken.text, NULL, 10));
        match(parser, lexer, TOK_NUMBER);
    } else if(checktoken(parser, TOK_CHAR)) {
        emitbyte(&emitter, 0x02); // DAT
        emitbyte32(&emitter, (char)parser->curtoken.text[0]);
        match(parser, lexer, TOK_CHAR);
    } else if(checktoken(parser, TOK_LABEL)) {
        emitbyte(&emitter, 0x02); // DAT
        parsesymbol(parser, lexer);
    } else if(checktoken(parser, TOK_DOLLAR)) {
        emitbyte(&emitter, 0x02); // DAT
        emitbyte32(&emitter, start + emitter.offset);
        match(parser, lexer, TOK_DOLLAR);
    }
}

static void parsejnz(struct Parser *parser, struct Lexer *lexer) {
    uint32_t start = emitter.written;
    emitbyte(&emitter, 0x05); // JNZ

    // DEST
    if(checktoken(parser, TOK_NUMBER))  {
        emitbyte(&emitter, 0x02);
        emitbyte32(&emitter, strtol(parser->curtoken.text, NULL, 10));
        match(parser, lexer, TOK_NUMBER);
    } else if(checktoken(parser, TOK_CHAR)) {
        emitbyte(&emitter, 0x02); // DAT
        emitbyte32(&emitter, (char)parser->curtoken.text[0]);
        match(parser, lexer, TOK_CHAR);
    } else if(checktoken(parser, TOK_LABEL)) {
        emitbyte(&emitter, 0x02); // DAT
        parsesymbol(parser, lexer);
    } else if(checktoken(parser, TOK_REGISTER)) {
        emitbyte(&emitter, 0x00); // REG
        emitbyte(&emitter, resolveregister(parser->curtoken.text));
        match(parser, lexer, TOK_REGISTER);
    } else if(checktoken(parser, TOK_LSQUARE)) {
        emitbyte(&emitter, 0x01); // PTR
        parseptr(parser, lexer);
    } else if(checktoken(parser, TOK_DOLLAR)) {
        emitbyte(&emitter, 0x02); // DAT
        emitbyte32(&emitter, start + emitter.offset);
        match(parser, lexer, TOK_DOLLAR);
    }
}

static void parsejz(struct Parser *parser, struct Lexer *lexer) {
    uint32_t start = emitter.written;
    emitbyte(&emitter, 0x06); // JZ
    // DEST
    if(checktoken(parser, TOK_NUMBER))  {
        emitbyte(&emitter, 0x02);
        emitbyte32(&emitter, strtol(parser->curtoken.text, NULL, 10));
        match(parser, lexer, TOK_NUMBER);
    } else if(checktoken(parser, TOK_CHAR)) {
        emitbyte(&emitter, 0x02); // DAT
        emitbyte32(&emitter, (char)parser->curtoken.text[0]);
        match(parser, lexer, TOK_CHAR);
    } else if(checktoken(parser, TOK_LABEL)) {
        emitbyte(&emitter, 0x02); // DAT
        parsesymbol(parser, lexer);
    } else if(checktoken(parser, TOK_REGISTER)) {
        emitbyte(&emitter, 0x00); // REG
        emitbyte(&emitter, resolveregister(parser->curtoken.text));
        match(parser, lexer, TOK_REGISTER);
    } else if(checktoken(parser, TOK_LSQUARE)) {
        emitbyte(&emitter, 0x01); // PTR
        parseptr(parser, lexer);
    } else if(checktoken(parser, TOK_DOLLAR)) {
        emitbyte(&emitter, 0x02); // DAT
        emitbyte32(&emitter, start + emitter.offset);
        match(parser, lexer, TOK_DOLLAR);
    }
}

static void parsejl(struct Parser *parser, struct Lexer *lexer) {
    uint32_t start = emitter.written;
    emitbyte(&emitter, 0x19); // JL
    // DEST
    if(checktoken(parser, TOK_NUMBER))  {
        emitbyte(&emitter, 0x02);
        emitbyte32(&emitter, strtol(parser->curtoken.text, NULL, 10));
        match(parser, lexer, TOK_NUMBER);
    } else if(checktoken(parser, TOK_CHAR)) {
        emitbyte(&emitter, 0x02); // DAT
        emitbyte32(&emitter, (char)parser->curtoken.text[0]);
        match(parser, lexer, TOK_CHAR);
    } else if(checktoken(parser, TOK_LABEL)) {
        emitbyte(&emitter, 0x02); // DAT
        parsesymbol(parser, lexer);
    } else if(checktoken(parser, TOK_REGISTER)) {
        emitbyte(&emitter, 0x00); // REG
        emitbyte(&emitter, resolveregister(parser->curtoken.text));
        match(parser, lexer, TOK_REGISTER);
    } else if(checktoken(parser, TOK_LSQUARE)) {
        emitbyte(&emitter, 0x01); // PTR
        parseptr(parser, lexer);
    }  else if(checktoken(parser, TOK_DOLLAR)) {
        emitbyte(&emitter, 0x02); // DAT
        emitbyte32(&emitter, start + emitter.offset);
        match(parser, lexer, TOK_DOLLAR);
    }
}


static void parsejle(struct Parser *parser, struct Lexer *lexer) {
    uint32_t start = emitter.written;
    emitbyte(&emitter, 0x1A); // JLE
    // DEST
    if(checktoken(parser, TOK_NUMBER))  {
        emitbyte(&emitter, 0x02);
        emitbyte32(&emitter, strtol(parser->curtoken.text, NULL, 10));
        match(parser, lexer, TOK_NUMBER);
    } else if(checktoken(parser, TOK_CHAR)) {
        emitbyte(&emitter, 0x02); // DAT
        emitbyte32(&emitter, (char)parser->curtoken.text[0]);
        match(parser, lexer, TOK_CHAR);
    } else if(checktoken(parser, TOK_LABEL)) {
        emitbyte(&emitter, 0x02); // DAT
        parsesymbol(parser, lexer);
    } else if(checktoken(parser, TOK_REGISTER)) {
        emitbyte(&emitter, 0x00); // REG
        emitbyte(&emitter, resolveregister(parser->curtoken.text));
        match(parser, lexer, TOK_REGISTER);
    } else if(checktoken(parser, TOK_LSQUARE)) {
        emitbyte(&emitter, 0x01); // PTR
        parseptr(parser, lexer);
    } else if(checktoken(parser, TOK_DOLLAR)) {
        emitbyte(&emitter, 0x02); // DAT
        emitbyte32(&emitter, start + emitter.offset);
        match(parser, lexer, TOK_DOLLAR);
    }
}

static void parsejg(struct Parser *parser, struct Lexer *lexer) {
    uint32_t start = emitter.written;
    emitbyte(&emitter, 0x1B); // JG
    // DEST
    if(checktoken(parser, TOK_NUMBER))  {
        emitbyte(&emitter, 0x02);
        emitbyte32(&emitter, strtol(parser->curtoken.text, NULL, 10));
        match(parser, lexer, TOK_NUMBER);
    } else if(checktoken(parser, TOK_CHAR)) {
        emitbyte(&emitter, 0x02); // DAT
        emitbyte32(&emitter, (char)parser->curtoken.text[0]);
        match(parser, lexer, TOK_CHAR);
    } else if(checktoken(parser, TOK_LABEL)) {
        emitbyte(&emitter, 0x02); // DAT
        parsesymbol(parser, lexer);
    } else if(checktoken(parser, TOK_REGISTER)) {
        emitbyte(&emitter, 0x00); // REG
        emitbyte(&emitter, resolveregister(parser->curtoken.text));
        match(parser, lexer, TOK_REGISTER);
    } else if(checktoken(parser, TOK_LSQUARE)) {
        emitbyte(&emitter, 0x01); // PTR
        parseptr(parser, lexer);
    } else if(checktoken(parser, TOK_DOLLAR)) {
        emitbyte(&emitter, 0x02); // DAT
        emitbyte32(&emitter, start + emitter.offset);
        match(parser, lexer, TOK_DOLLAR);
    }
}

static void parsejge(struct Parser *parser, struct Lexer *lexer) {
    uint32_t start = emitter.written;
    emitbyte(&emitter, 0x1C); // JGE
    // DEST
    if(checktoken(parser, TOK_NUMBER))  {
        emitbyte(&emitter, 0x02);
        emitbyte32(&emitter, strtol(parser->curtoken.text, NULL, 10));
        match(parser, lexer, TOK_NUMBER);
    } else if(checktoken(parser, TOK_CHAR)) {
        emitbyte(&emitter, 0x02); // DAT
        emitbyte32(&emitter, (char)parser->curtoken.text[0]);
        match(parser, lexer, TOK_CHAR);
    } else if(checktoken(parser, TOK_LABEL)) {
        emitbyte(&emitter, 0x02); // DAT
        parsesymbol(parser, lexer);
    } else if(checktoken(parser, TOK_REGISTER)) {
        emitbyte(&emitter, 0x00); // REG
        emitbyte(&emitter, resolveregister(parser->curtoken.text));
        match(parser, lexer, TOK_REGISTER);
    } else if(checktoken(parser, TOK_LSQUARE)) {
        emitbyte(&emitter, 0x01); // PTR
        parseptr(parser, lexer);
    } else if(checktoken(parser, TOK_DOLLAR)) {
        emitbyte(&emitter, 0x02); // DAT
        emitbyte32(&emitter, start + emitter.offset);
        match(parser, lexer, TOK_DOLLAR);
    }

}

static void parseseteq(struct Parser *parser, struct Lexer *lexer) {
    uint32_t start = emitter.written;
    emitbyte(&emitter, 0x1D); // SET
    // DEST
    if(checktoken(parser, TOK_REGISTER)) {
        emitbyte(&emitter, 0x00); // REG
        emitbyte(&emitter, resolveregister(parser->curtoken.text));
        match(parser, lexer, TOK_REGISTER);
    } else if(checktoken(parser, TOK_LSQUARE)) {
        emitbyte(&emitter, 0x01); // PTR
        parseptr(parser, lexer);
    }

}

static void parsesetne(struct Parser *parser, struct Lexer *lexer) {
    uint32_t start = emitter.written;
    emitbyte(&emitter, 0x1D); // SET
    // DEST
    if(checktoken(parser, TOK_REGISTER)) {
        emitbyte(&emitter, 0x02); // REG
        emitbyte(&emitter, resolveregister(parser->curtoken.text));
        match(parser, lexer, TOK_REGISTER);
    } else if(checktoken(parser, TOK_LSQUARE)) {
        emitbyte(&emitter, 0x03); // PTR
        parseptr(parser, lexer);
    }

}

static void parsesetlt(struct Parser *parser, struct Lexer *lexer) {
    uint32_t start = emitter.written;
    emitbyte(&emitter, 0x1D); // SET
    // DEST
    if(checktoken(parser, TOK_REGISTER)) {
        emitbyte(&emitter, 0x04); // REG
        emitbyte(&emitter, resolveregister(parser->curtoken.text));
        match(parser, lexer, TOK_REGISTER);
    } else if(checktoken(parser, TOK_LSQUARE)) {
        emitbyte(&emitter, 0x05); // PTR
        parseptr(parser, lexer);
    }

}

static void parsesetgt(struct Parser *parser, struct Lexer *lexer) {
    uint32_t start = emitter.written;
    emitbyte(&emitter, 0x1D); // SET
    // DEST
    if(checktoken(parser, TOK_REGISTER)) {
        emitbyte(&emitter, 0x06); // REG
        emitbyte(&emitter, resolveregister(parser->curtoken.text));
        match(parser, lexer, TOK_REGISTER);
    } else if(checktoken(parser, TOK_LSQUARE)) {
        emitbyte(&emitter, 0x07); // PTR
        parseptr(parser, lexer);
    }

}

static void parsesetle(struct Parser *parser, struct Lexer *lexer) {
    uint32_t start = emitter.written;
    emitbyte(&emitter, 0x1D); // SET
    // DEST
    if(checktoken(parser, TOK_REGISTER)) {
        emitbyte(&emitter, 0x08); // REG
        emitbyte(&emitter, resolveregister(parser->curtoken.text));
        match(parser, lexer, TOK_REGISTER);
    } else if(checktoken(parser, TOK_LSQUARE)) {
        emitbyte(&emitter, 0x09); // PTR
        parseptr(parser, lexer);
    }

}

static void parsesetge(struct Parser *parser, struct Lexer *lexer) {
    uint32_t start = emitter.written;
    emitbyte(&emitter, 0x1D); // SET
    // DEST
    if(checktoken(parser, TOK_REGISTER)) {
        emitbyte(&emitter, 0x0A); // REG
        emitbyte(&emitter, resolveregister(parser->curtoken.text));
        match(parser, lexer, TOK_REGISTER);
    } else if(checktoken(parser, TOK_LSQUARE)) {
        emitbyte(&emitter, 0x0B); // PTR
        parseptr(parser, lexer);
    }

}

static void parsecmp(struct Parser *parser, struct Lexer *lexer) {
    emitbyte(&emitter, 0x12); // CMP
    uint32_t loc = emitter.written;
    emitbyte(&emitter, 0x00); // mock mode
    uint8_t mode = 0x00; // default (REGREG)
    // DEST, SRC
    if(checktoken(parser, TOK_NUMBER))  {
        mode = 0x06; // DATXXX
        emitbyte32(&emitter, strtol(parser->curtoken.text, NULL, 10));
        match(parser, lexer, TOK_NUMBER);
    } else if(checktoken(parser, TOK_CHAR)) {
        mode = 0x06; // DATXXX
        emitbyte32(&emitter, (char)parser->curtoken.text[0]);
        match(parser, lexer, TOK_CHAR);
    } else if(checktoken(parser, TOK_LABEL)) {
        mode = 0x06; // DATXXX
        parsesymbol(parser, lexer);
    } else if(checktoken(parser, TOK_REGISTER)) {
        mode = 0x00; // REGXXX
        emitbyte(&emitter, resolveregister(parser->curtoken.text));
        match(parser, lexer, TOK_REGISTER);
    } else if(checktoken(parser, TOK_LSQUARE)) {
        mode = 0x03; // PTRXXX
        parseptr(parser, lexer);
    }

    match(parser, lexer, TOK_COMMA);

    if(checktoken(parser, TOK_NUMBER))  {
        mode += 2; // XXXDAT
        emitbyte32(&emitter, strtol(parser->curtoken.text, NULL, 10));
        match(parser, lexer, TOK_NUMBER);
    } else if(checktoken(parser, TOK_CHAR)) {
        mode += 2; // XXXDAT
        emitbyte32(&emitter, (char)parser->curtoken.text[0]);
        match(parser, lexer, TOK_CHAR);
    } else if(checktoken(parser, TOK_LABEL)) {
        mode += 2; // XXXDAT
        parsesymbol(parser, lexer);
    } else if(checktoken(parser, TOK_REGISTER)) {
        mode += 0; // XXXREG
        emitbyte(&emitter, resolveregister(parser->curtoken.text));
        match(parser, lexer, TOK_REGISTER);
    } else if(checktoken(parser, TOK_LSQUARE)) {
        mode += 1; // XXXPTR
        parseptr(parser, lexer);
    }

    relocatebyte(&emitter, loc, mode); // set mode
}

static void parseint(struct Parser *parser, struct Lexer *lexer) {
    emitbyte(&emitter, 0x03); // INT
    if(checktoken(parser, TOK_NUMBER)) {
        emitbyte(&emitter, 0x00);
        emitbyte32(&emitter, strtol(parser->curtoken.text, NULL, 10));
        match(parser, lexer, TOK_NUMBER);
    } else if(checktoken(parser, TOK_CHAR)) {
        emitbyte(&emitter, 0x00);
        emitbyte32(&emitter, (char)parser->curtoken.text[0]);
        match(parser, lexer, TOK_CHAR);
    } else if(checktoken(parser, TOK_LABEL)) {
        emitbyte(&emitter, 0x00);
        parsesymbol(parser, lexer);
    } else if(checktoken(parser, TOK_REGISTER)) {
        emitbyte(&emitter, 0x01);
        emitbyte(&emitter, resolveregister(parser->curtoken.text));
        match(parser, lexer, TOK_REGISTER);
    } else if(checktoken(parser, TOK_LSQUARE)) {
        emitbyte(&emitter, 0x02);
        parseptr(parser, lexer);
    }
}

static void parsecall(struct Parser *parser, struct Lexer *lexer) {
    emitbyte(&emitter, 0x07); // CALL
    if(checktoken(parser, TOK_NUMBER)) {
        emitbyte(&emitter, 0x00); // DEFAULT
        emitbyte32(&emitter, strtol(parser->curtoken.text, NULL, 10));
        match(parser, lexer, TOK_NUMBER);
    } else if(checktoken(parser, TOK_CHAR)) {
        emitbyte(&emitter, 0x00); // DEFAULT
        emitbyte32(&emitter, (char)parser->curtoken.text[0]);
        match(parser, lexer, TOK_CHAR);
    } else if(checktoken(parser, TOK_LABEL)) {
        emitbyte(&emitter, 0x00); // DEFAULT
        parsesymbol(parser, lexer);
    } else if(checktoken(parser, TOK_LSQUARE)) {
        emitbyte(&emitter, 0x01); // PTR
        parseptr(parser, lexer);
    }
}

static void parseinx(struct Parser *parser, struct Lexer *lexer) {
    emitbyte(&emitter, 0x09); // INX
    if(checktoken(parser, TOK_NUMBER)) {
        emitbyte(&emitter, 0x02); // DAT
        emitbyte32(&emitter, strtol(parser->curtoken.text, NULL, 10));
        match(parser, lexer, TOK_NUMBER);
    } else if(checktoken(parser, TOK_CHAR)) {
        emitbyte(&emitter, 0x02); // DAT
        emitbyte32(&emitter, (char)parser->curtoken.text[0]);
        match(parser, lexer, TOK_CHAR);
    } else if(checktoken(parser, TOK_LABEL)) {
        emitbyte(&emitter, 0x02); // DAT
        parsesymbol(parser, lexer);
    } else if(checktoken(parser, TOK_REGISTER)) {
        emitbyte(&emitter, 0x00); // REG
        emitbyte(&emitter, resolveregister(parser->curtoken.text));
        match(parser, lexer, TOK_REGISTER);
    } else if(checktoken(parser, TOK_LSQUARE)) {
        emitbyte(&emitter, 0x01); // PTR
        parseptr(parser, lexer);
    }
}

static void parseoutx(struct Parser *parser, struct Lexer *lexer) {
    emitbyte(&emitter, 0x0A); // OUTX
    uint32_t loc = emitter.written;
    emitbyte(&emitter, 0x00); // mock mode
    uint8_t mode = 0x00; // default (REGREG)
    // DEST, SRC
    if(checktoken(parser, TOK_NUMBER))  {
        mode = 0x06; // DATXXX
        emitbyte32(&emitter, strtol(parser->curtoken.text, NULL, 10));
        match(parser, lexer, TOK_NUMBER);
    } else if(checktoken(parser, TOK_CHAR)) {
        mode = 0x06; // DATXXX
        emitbyte32(&emitter, (char)parser->curtoken.text[0]);
        match(parser, lexer, TOK_CHAR);
    } else if(checktoken(parser, TOK_LABEL)) {
        mode = 0x06; // DATXXX
        parsesymbol(parser, lexer);
    } else if(checktoken(parser, TOK_REGISTER)) {
        mode = 0x00; // REGXXX
        emitbyte(&emitter, resolveregister(parser->curtoken.text));
        match(parser, lexer, TOK_REGISTER);
    } else if(checktoken(parser, TOK_LSQUARE)) {
        mode = 0x03; // PTRXXX
        parseptr(parser, lexer);
    }

    match(parser, lexer, TOK_COMMA);

    if(checktoken(parser, TOK_NUMBER))  {
        mode += 2; // XXXDAT
        emitbyte32(&emitter, strtol(parser->curtoken.text, NULL, 10));
        match(parser, lexer, TOK_NUMBER);
    } else if(checktoken(parser, TOK_CHAR)) {
        mode += 2; // XXXDAT
        emitbyte32(&emitter, (char)parser->curtoken.text[0]);
        match(parser, lexer, TOK_CHAR);
    } else if(checktoken(parser, TOK_LABEL)) {
        mode += 2; // XXXDAT
        parsesymbol(parser, lexer);
    } else if(checktoken(parser, TOK_REGISTER)) {
        mode += 0; // XXXREG
        emitbyte(&emitter, resolveregister(parser->curtoken.text));
        match(parser, lexer, TOK_REGISTER);
    } else if(checktoken(parser, TOK_LSQUARE)) {
        mode += 1; // XXXPTR
        parseptr(parser, lexer);
    }

    relocatebyte(&emitter, loc, mode); // set mode
}

static void parsepop(struct Parser *parser, struct Lexer *lexer) {
    emitbyte(&emitter, 0x0B); // POP
    if(checktoken(parser, TOK_REGISTER)) {
        emitbyte(&emitter, 0x00); // REG
        emitbyte(&emitter, resolveregister(parser->curtoken.text));
        match(parser, lexer, TOK_REGISTER);
    } else if(checktoken(parser, TOK_LSQUARE)) {
        emitbyte(&emitter, 0x01); // PTR
        parseptr(parser, lexer);
    }
}

static void parsepush(struct Parser *parser, struct Lexer *lexer) {
    emitbyte(&emitter, 0x0C); // PUSH
    if(checktoken(parser, TOK_NUMBER)) {
        emitbyte(&emitter, 0x02); // DAT
        emitbyte32(&emitter, strtol(parser->curtoken.text, NULL, 10));
        match(parser, lexer, TOK_NUMBER);
    } else if(checktoken(parser, TOK_CHAR)) {
        emitbyte(&emitter, 0x02); // DAT
        emitbyte32(&emitter, (char)parser->curtoken.text[0]);
        match(parser, lexer, TOK_CHAR);
    } else if(checktoken(parser, TOK_LABEL)) {
        emitbyte(&emitter, 0x02); // DAT
        parsesymbol(parser, lexer);
    } else if(checktoken(parser, TOK_REGISTER)) {
        emitbyte(&emitter, 0x00); // REG
        emitbyte(&emitter, resolveregister(parser->curtoken.text));
        match(parser, lexer, TOK_REGISTER);
    } else if(checktoken(parser, TOK_LSQUARE)) {
        emitbyte(&emitter, 0x01); // PTR
        parseptr(parser, lexer);
    }

}

static void parseadd(struct Parser *parser, struct Lexer *lexer) {
    emitbyte(&emitter, 0x0D); // ADD
    uint32_t loc = emitter.written;
    emitbyte(&emitter, 0x00); // mock mode
    uint8_t mode = 0x00; // default (REGREG)
    // DEST, SRC
    if(checktoken(parser, TOK_REGISTER)) {
        mode = 0x00; // REGXXX
        emitbyte(&emitter, resolveregister(parser->curtoken.text));
        match(parser, lexer, TOK_REGISTER);
    } else if(checktoken(parser, TOK_LSQUARE)) {
        mode = 0x03; // PTRXXX
        parseptr(parser, lexer);
    }

    match(parser, lexer, TOK_COMMA);

    if(checktoken(parser, TOK_NUMBER))  {
        mode += 2; // XXXDAT
        emitbyte32(&emitter, strtol(parser->curtoken.text, NULL, 10));
        match(parser, lexer, TOK_NUMBER);
    } else if(checktoken(parser, TOK_CHAR)) {
        mode += 2; // XXXDAT
        emitbyte32(&emitter, (char)parser->curtoken.text[0]);
        match(parser, lexer, TOK_CHAR);
    } else if(checktoken(parser, TOK_LABEL)) {
        mode += 2; // XXXDAT
        parsesymbol(parser, lexer);
    } else if(checktoken(parser, TOK_REGISTER)) {
        mode += 0; // XXXREG
        emitbyte(&emitter, resolveregister(parser->curtoken.text));
        match(parser, lexer, TOK_REGISTER);
    } else if(checktoken(parser, TOK_LSQUARE)) {
        mode += 1; // XXXPTR
        parseptr(parser, lexer);
    }

    relocatebyte(&emitter, loc, mode); // set mode
}

static void parsesub(struct Parser *parser, struct Lexer *lexer) {
    emitbyte(&emitter, 0x0E); // SUB
    uint32_t loc = emitter.written;
    emitbyte(&emitter, 0x00); // mock mode
    uint8_t mode = 0x00; // default (REGREG)
    // DEST, SRC
    if(checktoken(parser, TOK_REGISTER)) {
        mode = 0x00; // REGXXX
        emitbyte(&emitter, resolveregister(parser->curtoken.text));
        match(parser, lexer, TOK_REGISTER);
    } else if(checktoken(parser, TOK_LSQUARE)) {
        mode = 0x03; // PTRXXX
        parseptr(parser, lexer);
    }

    match(parser, lexer, TOK_COMMA);

    if(checktoken(parser, TOK_NUMBER))  {
        mode += 2; // XXXDAT
        emitbyte32(&emitter, strtol(parser->curtoken.text, NULL, 10));
        match(parser, lexer, TOK_NUMBER);
    } else if(checktoken(parser, TOK_CHAR)) {
        mode += 2; // XXXDAT
        emitbyte32(&emitter, (char)parser->curtoken.text[0]);
        match(parser, lexer, TOK_CHAR);
    } else if(checktoken(parser, TOK_LABEL)) {
        mode += 2; // XXXDAT
        parsesymbol(parser, lexer);
    } else if(checktoken(parser, TOK_REGISTER)) {
        mode += 0; // XXXREG
        emitbyte(&emitter, resolveregister(parser->curtoken.text));
        match(parser, lexer, TOK_REGISTER);
    } else if(checktoken(parser, TOK_LSQUARE)) {
        mode += 1; // XXXPTR
        parseptr(parser, lexer);
    }

    relocatebyte(&emitter, loc, mode); // set mode
}

static void parsediv(struct Parser *parser, struct Lexer *lexer) {
    emitbyte(&emitter, 0x0F); // DIV
    uint32_t loc = emitter.written;
    emitbyte(&emitter, 0x00); // mock mode
    uint8_t mode = 0x00; // default (REGREG)
    // DEST, SRC
    if(checktoken(parser, TOK_REGISTER)) {
        mode = 0x00; // REGXXX
        emitbyte(&emitter, resolveregister(parser->curtoken.text));
        match(parser, lexer, TOK_REGISTER);
    } else if(checktoken(parser, TOK_LSQUARE)) {
        mode = 0x03; // PTRXXX
        parseptr(parser, lexer);
    }

    match(parser, lexer, TOK_COMMA);

    if(checktoken(parser, TOK_NUMBER))  {
        mode += 2; // XXXDAT
        emitbyte32(&emitter, strtol(parser->curtoken.text, NULL, 10));
        match(parser, lexer, TOK_NUMBER);
    } else if(checktoken(parser, TOK_CHAR)) {
        mode += 2; // XXXDAT
        emitbyte32(&emitter, (char)parser->curtoken.text[0]);
        match(parser, lexer, TOK_CHAR);
    } else if(checktoken(parser, TOK_LABEL)) {
        mode += 2; // XXXDAT
        parsesymbol(parser, lexer);
    } else if(checktoken(parser, TOK_REGISTER)) {
        mode += 0; // XXXREG
        emitbyte(&emitter, resolveregister(parser->curtoken.text));
        match(parser, lexer, TOK_REGISTER);
    } else if(checktoken(parser, TOK_LSQUARE)) {
        mode += 1; // XXXPTR
        parseptr(parser, lexer);
    }

    relocatebyte(&emitter, loc, mode); // set mode
}

static void parseidiv(struct Parser *parser, struct Lexer *lexer) {
    emitbyte(&emitter, 0x10); // IDIV
    uint32_t loc = emitter.written;
    emitbyte(&emitter, 0x00); // mock mode
    uint8_t mode = 0x00; // default (REGREG)
    // DEST, SRC
    if(checktoken(parser, TOK_REGISTER)) {
        mode = 0x00; // REGXXX
        emitbyte(&emitter, resolveregister(parser->curtoken.text));
        match(parser, lexer, TOK_REGISTER);
    } else if(checktoken(parser, TOK_LSQUARE)) {
        mode = 0x03; // PTRXXX
        parseptr(parser, lexer);
    }

    match(parser, lexer, TOK_COMMA);

    if(checktoken(parser, TOK_NUMBER))  {
        mode += 2; // XXXDAT
        emitbyte32(&emitter, strtol(parser->curtoken.text, NULL, 10));
        match(parser, lexer, TOK_NUMBER);
    } else if(checktoken(parser, TOK_CHAR)) {
        mode += 2; // XXXDAT
        emitbyte32(&emitter, (char)parser->curtoken.text[0]);
        match(parser, lexer, TOK_CHAR);
    } else if(checktoken(parser, TOK_LABEL)) {
        mode += 2; // XXXDAT
        parsesymbol(parser, lexer);
    } else if(checktoken(parser, TOK_REGISTER)) {
        mode += 0; // XXXREG
        emitbyte(&emitter, resolveregister(parser->curtoken.text));
        match(parser, lexer, TOK_REGISTER);
    } else if(checktoken(parser, TOK_LSQUARE)) {
        mode += 1; // XXXPTR
        parseptr(parser, lexer);
    }

    relocatebyte(&emitter, loc, mode); // set mode
}

static void parsemul(struct Parser *parser, struct Lexer *lexer) {
    emitbyte(&emitter, 0x11); // MUL
    uint32_t loc = emitter.written;
    emitbyte(&emitter, 0x00); // mock mode
    uint8_t mode = 0x00; // default (REGREG)
    // DEST, SRC
    if(checktoken(parser, TOK_REGISTER)) {
        mode = 0x00; // REGXXX
        emitbyte(&emitter, resolveregister(parser->curtoken.text));
        match(parser, lexer, TOK_REGISTER);
    } else if(checktoken(parser, TOK_LSQUARE)) {
        mode = 0x03; // PTRXXX
        parseptr(parser, lexer);
    }

    match(parser, lexer, TOK_COMMA);

    if(checktoken(parser, TOK_NUMBER))  {
        mode += 2; // XXXDAT
        emitbyte32(&emitter, strtol(parser->curtoken.text, NULL, 10));
        match(parser, lexer, TOK_NUMBER);
    } else if(checktoken(parser, TOK_CHAR)) {
        mode += 2; // XXXDAT
        emitbyte32(&emitter, (char)parser->curtoken.text[0]);
        match(parser, lexer, TOK_CHAR);
    } else if(checktoken(parser, TOK_LABEL)) {
        mode += 2; // XXXDAT
        parsesymbol(parser, lexer);
    } else if(checktoken(parser, TOK_REGISTER)) {
        mode += 0; // XXXREG
        emitbyte(&emitter, resolveregister(parser->curtoken.text));
        match(parser, lexer, TOK_REGISTER);
    } else if(checktoken(parser, TOK_LSQUARE)) {
        mode += 1; // XXXPTR
        parseptr(parser, lexer);
    }

    relocatebyte(&emitter, loc, mode); // set mode
}

static void parseand(struct Parser *parser, struct Lexer *lexer) {
    emitbyte(&emitter, 0x13); // AND
    uint32_t loc = emitter.written;
    emitbyte(&emitter, 0x00); // mock mode
    uint8_t mode = 0x00; // default (REGREG)
    // DEST, SRC
    if(checktoken(parser, TOK_REGISTER)) {
        mode = 0x00; // REGXXX
        emitbyte(&emitter, resolveregister(parser->curtoken.text));
        match(parser, lexer, TOK_REGISTER);
    } else if(checktoken(parser, TOK_LSQUARE)) {
        mode = 0x03; // PTRXXX
        parseptr(parser, lexer);
    }

    match(parser, lexer, TOK_COMMA);

    if(checktoken(parser, TOK_NUMBER))  {
        mode += 2; // XXXDAT
        emitbyte32(&emitter, strtol(parser->curtoken.text, NULL, 10));
        match(parser, lexer, TOK_NUMBER);
    } else if(checktoken(parser, TOK_CHAR)) {
        mode += 2; // XXXDAT
        emitbyte32(&emitter, (char)parser->curtoken.text[0]);
        match(parser, lexer, TOK_CHAR);
    } else if(checktoken(parser, TOK_LABEL)) {
        mode += 2; // XXXDAT
        parsesymbol(parser, lexer);
    } else if(checktoken(parser, TOK_REGISTER)) {
        mode += 0; // XXXREG
        emitbyte(&emitter, resolveregister(parser->curtoken.text));
        match(parser, lexer, TOK_REGISTER);
    } else if(checktoken(parser, TOK_LSQUARE)) {
        mode += 1; // XXXPTR
        parseptr(parser, lexer);
    }

    relocatebyte(&emitter, loc, mode); // set mode
}

static void parseshl(struct Parser *parser, struct Lexer *lexer) {
    emitbyte(&emitter, 0x14); // SHL
    uint32_t loc = emitter.written;
    emitbyte(&emitter, 0x00); // mock mode
    uint8_t mode = 0x00; // default (REGREG)
    // DEST, SRC
    if(checktoken(parser, TOK_REGISTER)) {
        mode = 0x00; // REGXXX
        emitbyte(&emitter, resolveregister(parser->curtoken.text));
        match(parser, lexer, TOK_REGISTER);
    } else if(checktoken(parser, TOK_LSQUARE)) {
        mode = 0x03; // PTRXXX
        parseptr(parser, lexer);
    }

    match(parser, lexer, TOK_COMMA);

    if(checktoken(parser, TOK_NUMBER))  {
        mode += 2; // XXXDAT
        emitbyte32(&emitter, strtol(parser->curtoken.text, NULL, 10));
        match(parser, lexer, TOK_NUMBER);
    } else if(checktoken(parser, TOK_CHAR)) {
        mode += 2; // XXXDAT
        emitbyte32(&emitter, (char)parser->curtoken.text[0]);
        match(parser, lexer, TOK_CHAR);
    } else if(checktoken(parser, TOK_LABEL)) {
        mode += 2; // XXXDAT
        parsesymbol(parser, lexer);
    } else if(checktoken(parser, TOK_REGISTER)) {
        mode += 0; // XXXREG
        emitbyte(&emitter, resolveregister(parser->curtoken.text));
        match(parser, lexer, TOK_REGISTER);
    } else if(checktoken(parser, TOK_LSQUARE)) {
        mode += 1; // XXXPTR
        parseptr(parser, lexer);
    }

    relocatebyte(&emitter, loc, mode); // set mode
}

static void parseshr(struct Parser *parser, struct Lexer *lexer) {
    emitbyte(&emitter, 0x15); // SHR
    uint32_t loc = emitter.written;
    emitbyte(&emitter, 0x00); // mock mode
    uint8_t mode = 0x00; // default (REGREG)
    // DEST, SRC
    if(checktoken(parser, TOK_REGISTER)) {
        mode = 0x00; // REGXXX
        emitbyte(&emitter, resolveregister(parser->curtoken.text));
        match(parser, lexer,TOK_REGISTER);
    } else if(checktoken(parser, TOK_LSQUARE)) {
        mode = 0x03; // PTRXXX
        parseptr(parser, lexer);
    }

    match(parser, lexer, TOK_COMMA);

    if(checktoken(parser, TOK_NUMBER))  {
        mode += 2; // XXXDAT
        emitbyte32(&emitter, strtol(parser->curtoken.text, NULL, 10));
        match(parser, lexer, TOK_NUMBER);
    } else if(checktoken(parser, TOK_CHAR)) {
        mode += 2; // XXXDAT
        emitbyte32(&emitter, (char)parser->curtoken.text[0]);
        match(parser, lexer, TOK_CHAR);
    } else if(checktoken(parser, TOK_LABEL)) {
        mode += 2; // XXXDAT
        parsesymbol(parser, lexer);
    } else if(checktoken(parser, TOK_REGISTER)) {
        mode += 0; // XXXREG
        emitbyte(&emitter, resolveregister(parser->curtoken.text));
        match(parser, lexer, TOK_REGISTER);
    } else if(checktoken(parser, TOK_LSQUARE)) {
        mode += 1; // XXXPTR
        parseptr(parser, lexer);
    }

    relocatebyte(&emitter, loc, mode); // set mode
}

static void parsexor(struct Parser *parser, struct Lexer *lexer) {
    emitbyte(&emitter, 0x16); // XOR
    uint32_t loc = emitter.written;
    emitbyte(&emitter, 0x00); // mock mode
    uint8_t mode = 0x00; // default (REGREG)
    // DEST, SRC
    if(checktoken(parser, TOK_REGISTER)) {
        mode = 0x00; // REGXXX
        emitbyte(&emitter, resolveregister(parser->curtoken.text));
        match(parser, lexer, TOK_REGISTER);
    } else if(checktoken(parser, TOK_LSQUARE)) {
        mode = 0x03; // PTRXXX
        parseptr(parser, lexer);
    }

    match(parser, lexer, TOK_COMMA);

    if(checktoken(parser, TOK_NUMBER))  {
        mode += 2; // XXXDAT
        emitbyte32(&emitter, strtol(parser->curtoken.text, NULL, 10));
        match(parser, lexer, TOK_NUMBER);
    } else if(checktoken(parser, TOK_CHAR)) {
        mode += 2; // XXXDAT
        emitbyte32(&emitter, (char)parser->curtoken.text[0]);
        match(parser, lexer, TOK_CHAR);
    } else if(checktoken(parser, TOK_LABEL)) {
        mode += 2; // XXXDAT
        parsesymbol(parser, lexer);
    } else if(checktoken(parser, TOK_REGISTER)) {
        mode += 0; // XXXREG
        emitbyte(&emitter, resolveregister(parser->curtoken.text));
        match(parser, lexer, TOK_REGISTER);
    } else if(checktoken(parser, TOK_LSQUARE)) {
        mode += 1; // XXXPTR
        parseptr(parser, lexer);
    }

    relocatebyte(&emitter, loc, mode); // set mode
}

static void parseor(struct Parser *parser, struct Lexer *lexer) {
    emitbyte(&emitter, 0x17); // OR
    uint32_t loc = emitter.written;
    emitbyte(&emitter, 0x00); // mock mode
    uint8_t mode = 0x00; // default (REGREG)
    // DEST, SRC
    if(checktoken(parser, TOK_REGISTER)) {
        mode = 0x00; // REGXXX
        emitbyte(&emitter, resolveregister(parser->curtoken.text));
        match(parser, lexer, TOK_REGISTER);
    } else if(checktoken(parser, TOK_LSQUARE)) {
        mode = 0x03; // PTRXXX
        parseptr(parser, lexer);
    }

    match(parser, lexer, TOK_COMMA);

    if(checktoken(parser, TOK_NUMBER))  {
        mode += 2; // XXXDAT
        emitbyte32(&emitter, strtol(parser->curtoken.text, NULL, 10));
        match(parser, lexer, TOK_NUMBER);
    } else if(checktoken(parser, TOK_CHAR)) {
        mode += 2; // XXXDAT
        emitbyte32(&emitter, (char)parser->curtoken.text[0]);
        match(parser, lexer, TOK_CHAR);
    } else if(checktoken(parser, TOK_LABEL)) {
        mode += 2; // XXXDAT
        parsesymbol(parser, lexer);
    } else if(checktoken(parser, TOK_REGISTER)) {
        mode += 0; // XXXREG
        emitbyte(&emitter, resolveregister(parser->curtoken.text));
        match(parser, lexer, TOK_REGISTER);
    } else if(checktoken(parser, TOK_LSQUARE)) {
        mode += 1; // XXXPTR
        parseptr(parser, lexer);
    }

    relocatebyte(&emitter, loc, mode); // set mode
}

static void parsenot(struct Parser *parser, struct Lexer *lexer) {
    emitbyte(&emitter, 0x18); // NOEG
    uint32_t loc = emitter.written;
    emitbyte(&emitter, 0x00); // mock mode
    uint8_t mode = 0x00; // default (REGREG)
    // DEST, SRC
    if(checktoken(parser, TOK_REGISTER)) {
        mode = 0x00; // REGXXX
        emitbyte(&emitter, resolveregister(parser->curtoken.text));
        match(parser, lexer, TOK_REGISTER);
    } else if(checktoken(parser, TOK_LSQUARE)) {
        mode = 0x01; // PTRXXX
        parseptr(parser, lexer);
    }

    relocatebyte(&emitter, loc, mode); // set mode
}

static void parseneg(struct Parser *parser, struct Lexer *lexer) {
    emitbyte(&emitter, 0x18); // NEG
    uint32_t loc = emitter.written;
    emitbyte(&emitter, 0x00); // mock mode
    uint8_t mode = 0x02; // default (REGREG)
    // DEST, SRC
    if(checktoken(parser, TOK_REGISTER)) {
        mode = 0x02; // REGXXX
        emitbyte(&emitter, resolveregister(parser->curtoken.text));
        match(parser, lexer, TOK_REGISTER);
    } else if(checktoken(parser, TOK_LSQUARE)) {
        mode = 0x03; // PTRXXX
        parseptr(parser, lexer);
    }

    relocatebyte(&emitter, loc, mode); // set mode
}

static void parselea(struct Parser *parser, struct Lexer *lexer) {
    emitbyte(&emitter, 0x1E); // LEA
    uint32_t loc = emitter.written;
    emitbyte(&emitter, 0x00); // mock mode
    uint8_t mode = 0x00; // default (REGREG)
    // DEST, SRC
    if(checktoken(parser, TOK_REGISTER)) {
        mode = 0x00; // REGXXX
        emitbyte(&emitter, resolveregister(parser->curtoken.text));
        match(parser, lexer, TOK_REGISTER);
    } else if(checktoken(parser, TOK_LSQUARE)) {
        mode = 0x01; // PTRXXX
        parseptr(parser, lexer);
    }

    match(parser, lexer, TOK_COMMA);

    if(checktoken(parser, TOK_LSQUARE)) {
        mode += 0; // XXXPTR
        parseptr(parser, lexer);
    }

    relocatebyte(&emitter, loc, mode); // set mode
}


static void parsetest(struct Parser *parser, struct Lexer *lexer) {
    emitbyte(&emitter, 0x13); // TEST
    uint32_t loc = emitter.written;
    emitbyte(&emitter, 0x00); // mock mode
    uint8_t mode = 0x00; // default (REGREG)
    // DEST, SRC
    if(checktoken(parser, TOK_REGISTER)) {
        mode = 0x06; // REGXXX
        emitbyte(&emitter, resolveregister(parser->curtoken.text));
        match(parser, lexer, TOK_REGISTER);
    } else if(checktoken(parser, TOK_LSQUARE)) {
        mode = 0x09; // PTRXXX
        parseptr(parser, lexer);
    }

    match(parser, lexer, TOK_COMMA);

    if(checktoken(parser, TOK_NUMBER))  {
        mode += 2; // XXXDAT
        emitbyte32(&emitter, strtol(parser->curtoken.text, NULL, 10));
        match(parser, lexer, TOK_NUMBER);
    } else if(checktoken(parser, TOK_CHAR)) {
        mode += 2; // XXXDAT
        emitbyte32(&emitter, (char)parser->curtoken.text[0]);
        match(parser, lexer, TOK_CHAR);
    } else if(checktoken(parser, TOK_LABEL)) {
        mode += 2; // XXXDAT
        parsesymbol(parser, lexer);
    } else if(checktoken(parser, TOK_REGISTER)) {
        mode += 0; // XXXREG
        emitbyte(&emitter, resolveregister(parser->curtoken.text));
        match(parser, lexer, TOK_REGISTER);
    } else if(checktoken(parser, TOK_LSQUARE)) {
        mode += 1; // XXXPTR
        parseptr(parser, lexer);
    }

    relocatebyte(&emitter, loc, mode); // set mode
}

// section
static int mode = 0; // text (0 text, 1 data)


// ---------
//  MACRO
// ---------

void compilefile(char *buffer);

static void parsemacro(struct Parser *parser, struct Lexer *lexer) {
    struct Token macroname = parser->curtoken;
    match(parser, lexer, TOK_LABEL); // expect macro operator
    if(strcmp(macroname.text, "define") == 0) { // constant
        struct Token constant = parser->curtoken;
        match(parser, lexer, TOK_LABEL);
        if(checktoken(parser, TOK_NUMBER)) {
            labelmapset(labels, constant.text, strtol(parser->curtoken.text, NULL, 10));
            match(parser, lexer, TOK_NUMBER);
        } else if(checktoken(parser, TOK_CHAR)) {
            labelmapset(labels, constant.text, (char)parser->curtoken.text[0]);
            match(parser, lexer, TOK_CHAR);
        }
    } else if(strcmp(macroname.text, "org") == 0) { // data origin
        if(checktoken(parser, TOK_NUMBER)) {
            emitter.cp = strtol(parser->curtoken.text, NULL, 10);
            if(emitter.cp > emitter.written) emitter.written = emitter.cp;
            match(parser, lexer, TOK_NUMBER);
        }
    } else if(strcmp(macroname.text, "include") == 0) { // include assembly file 
        struct Token constant = parser->curtoken;
        match(parser, lexer, TOK_STRING);
        FILE *file = fopen(constant.text, "r");

        fseek(file, 0, SEEK_END);
        size_t size = ftell(file);
        fseek(file, 0, SEEK_SET);

        char *buffer = (char *)malloc(size);

        size_t read = fread(buffer, sizeof(uint8_t), size, file);

        fclose(file);
        buffer[size] = '\0';
        printf("%zu\n", strlen(buffer));

        compilefile(buffer);

    } else if((strcmp(macroname.text, "incbin") == 0) && mode == 1) { // include a binary
        struct Token constant = parser->curtoken;
        match(parser, lexer, TOK_STRING);
        FILE *file = fopen(constant.text, "rb");
        fseek(file, 0, SEEK_END);
        size_t size = ftell(file);
        fseek(file, 0, SEEK_SET);

        uint8_t *buffer = (uint8_t *)malloc(size);

        size_t read = fread(buffer, sizeof(uint8_t), size, file);

        fclose(file);

        for(size_t i = 0; i < size; i++) {
            emitbyte(&emitter, buffer[i]);
        }

    }
}

// ---------
//  PERIOD
// ---------

static void parseperiod(struct Parser *parser, struct Lexer *lexer) {
    struct Token operator = parser->curtoken;
    match(parser, lexer, TOK_LABEL);
    if(strcmp(operator.text, "text") == 0) { // code
        mode = 0;
        return;
    } else if(strcmp(operator.text, "data") == 0) { // raw data
        mode = 1;
        return;
    } else if(strcmp(operator.text, "global") == 0) { // global (ignore for now)
        match(parser, lexer, TOK_LABEL); // symbol to make global :rolliong_eyes:
        return;
    }

    if(mode == 0) {
        // nothing to do
    } else {
    
        if(strcmp(operator.text, "byte") == 0) { // 8 bit integer
            if(checktoken(parser, TOK_NUMBER)) {
                emitbyte(&emitter, strtol(parser->curtoken.text, NULL, 10));
                match(parser, lexer, TOK_NUMBER);
            } else if(checktoken(parser, TOK_CHAR)) {
                emitbyte(&emitter, (char)parser->curtoken.text[0]);
                match(parser, lexer, TOK_CHAR);
            } else if(checktoken(parser, TOK_LABEL)) {
                parsesymbol(parser, lexer);
            }
        } else if(strcmp(operator.text, "word") == 0) { // 16 bit integer
            if(checktoken(parser, TOK_NUMBER)) {
                emitbyte16(&emitter, strtol(parser->curtoken.text, NULL, 10));
                match(parser, lexer, TOK_NUMBER);
            } else if(checktoken(parser, TOK_CHAR)) {
                emitbyte16(&emitter, (char)parser->curtoken.text[0]);
                match(parser, lexer, TOK_CHAR);
            } else if(checktoken(parser, TOK_LABEL)) {
                parsesymbol(parser, lexer);
            }
        } else if(strcmp(operator.text, "dword") == 0) { // 32 bit integer
            if(checktoken(parser, TOK_NUMBER)) {
                emitbyte32(&emitter, strtol(parser->curtoken.text, NULL, 10));
                match(parser, lexer, TOK_NUMBER);
            } else if(checktoken(parser, TOK_CHAR)) {
                emitbyte32(&emitter, (char)parser->curtoken.text[0]);
                match(parser, lexer, TOK_CHAR);
            } else if(checktoken(parser, TOK_LABEL)) {
                parsesymbol(parser, lexer);
            }
        } else if((strcmp(operator.text, "asciiz") == 0) || (strcmp(operator.text, "string") == 0)) { // NULL terminated string
            struct Token data = parser->curtoken;
            match(parser, lexer, TOK_STRING);
            for(size_t i = 0; i < strlen(data.text); i++) {
                emitbyte(&emitter, data.text[i]);
            }
            emitbyte(&emitter, 0x00); // null terminate
        } else if(strcmp(operator.text, "ascii") == 0) { // string (Not NULL terminated)
            struct Token data = parser->curtoken;
            match(parser, lexer, TOK_STRING);
            for(size_t i = 0; i < strlen(data.text); i++) {
                emitbyte(&emitter, data.text[i]);
            }
        }

    }
}

static void parsestatement(struct Parser *parser, struct Lexer *lexer) {
    // printf("Parsing/Assembling in mode 0x%02x\n", mode);
    if(mode == 1) { // data
        if(checktoken(parser, TOK_PERIOD)) { // actual operations
            // printf("period operation.\n");
            nexttoken(parser, lexer);
            parseperiod(parser, lexer);
        } else if(checktoken(parser, TOK_HASHTAG)) { // MACRO
            nexttoken(parser, lexer);
            parsemacro(parser, lexer);
        } else if(checktoken(parser, TOK_LABEL)) {
            char *label = parser->curtoken.text;
            // printf("Label: '%s', %d\n", label, parser->curtoken.type);
            labelmapset(labels, label, emitter.written + emitter.offset);
            nexttoken(parser, lexer);
            // printf("data mode parse colon\n");
            match(parser, lexer, TOK_COLON);
            if(checktoken(parser, TOK_NEWLINE)) nexttoken(parser, lexer);
            return;
        }

        checknewline(parser, lexer);
        return;
    }

    // effective ELSE
    if(checktoken(parser, TOK_NOP)) {
        nexttoken(parser, lexer);
        emitbyte(&emitter, 0x00);
        emitbyte(&emitter, 0x00);
    } else if(checktoken(parser, TOK_HLT)) {
        nexttoken(parser, lexer);
        emitbyte(&emitter, 0x01);
        emitbyte(&emitter, 0x00);
    } else if(checktoken(parser, TOK_MOV)) {
        nexttoken(parser, lexer);
        parsemov(parser, lexer);
    } else if(checktoken(parser, TOK_INT)) {
        nexttoken(parser, lexer);
        parseint(parser, lexer);
    } else if(checktoken(parser, TOK_JMP)) {
        nexttoken(parser, lexer);
        parsejmp(parser, lexer);
    } else if(checktoken(parser, TOK_JNZ)) {
        nexttoken(parser, lexer);
        parsejnz(parser, lexer);
    } else if(checktoken(parser, TOK_JZ)) {
        nexttoken(parser, lexer);
        parsejz(parser, lexer);
    } else if(checktoken(parser, TOK_JNE)) {
        nexttoken(parser, lexer);
        parsejnz(parser, lexer);
    } else if(checktoken(parser, TOK_JE)) {
        nexttoken(parser, lexer);
        parsejz(parser, lexer);
    } else if(checktoken(parser, TOK_JL)) {
        nexttoken(parser, lexer);
        parsejl(parser, lexer);
    } else if(checktoken(parser, TOK_JLE)) {
        nexttoken(parser, lexer);
        parsejle(parser, lexer);
    } else if(checktoken(parser, TOK_JG)) {
        nexttoken(parser, lexer);
        parsejg(parser, lexer);
    } else if(checktoken(parser, TOK_JGE)) {
        nexttoken(parser, lexer);
        parsejge(parser, lexer);
    } else if(checktoken(parser, TOK_SETEQ)) {
        nexttoken(parser, lexer);
        parseseteq(parser, lexer);
    } else if(checktoken(parser, TOK_SETNE)) {
        nexttoken(parser, lexer);
        parsesetne(parser, lexer);
    } else if(checktoken(parser, TOK_SETLT)) {
        nexttoken(parser, lexer);
        parsesetlt(parser, lexer);
    } else if(checktoken(parser, TOK_SETGT)) {
        nexttoken(parser, lexer);
        parsesetgt(parser, lexer);
    } else if(checktoken(parser, TOK_SETLE)) {
        nexttoken(parser, lexer);
        parsesetle(parser, lexer);
    } else if(checktoken(parser, TOK_SETGE)) {
        nexttoken(parser, lexer);
        parsesetge(parser, lexer);
    } else if(checktoken(parser, TOK_LEA)) {
        nexttoken(parser, lexer);
        parselea(parser, lexer);
    } else if(checktoken(parser, TOK_CMP)) {
        nexttoken(parser, lexer);
        parsecmp(parser, lexer);
    } else if(checktoken(parser, TOK_CALL)) {
        nexttoken(parser, lexer);
        parsecall(parser, lexer);
    } else if(checktoken(parser, TOK_RET)) {
        nexttoken(parser, lexer);
        emitbyte(&emitter, 0x08); // RET
        emitbyte(&emitter, 0x00);
    } else if(checktoken(parser, TOK_INX)) {
        nexttoken(parser, lexer);
        parseinx(parser, lexer);
    } else if(checktoken(parser, TOK_OUTX)) {
        nexttoken(parser, lexer);
        parseoutx(parser, lexer);
    } else if(checktoken(parser, TOK_POP)) {
        nexttoken(parser, lexer);
        parsepop(parser, lexer);
    } else if(checktoken(parser, TOK_PUSH)) {
        nexttoken(parser, lexer);
        parsepush(parser, lexer);
    } else if(checktoken(parser, TOK_ADD)) {
        nexttoken(parser, lexer);
        parseadd(parser, lexer); 
    } else if(checktoken(parser, TOK_SUB)) {
        nexttoken(parser, lexer);
        parsesub(parser, lexer);
    } else if(checktoken(parser, TOK_DIV)) {
        nexttoken(parser, lexer);
        parsediv(parser, lexer);
    } else if(checktoken(parser, TOK_IDIV)) {
        nexttoken(parser, lexer);
        parseidiv(parser, lexer);
    } else if(checktoken(parser, TOK_MUL)) {
        nexttoken(parser, lexer);
        parsemul(parser, lexer);
    } else if(checktoken(parser, TOK_AND)) {
        nexttoken(parser, lexer);
        parseand(parser, lexer);
    } else if(checktoken(parser, TOK_SHL)) {
        nexttoken(parser, lexer);
        parseshl(parser, lexer);
    } else if(checktoken(parser, TOK_SHR)) {
        nexttoken(parser, lexer);
        parseshr(parser, lexer);
    } else if(checktoken(parser, TOK_XOR)) {
        nexttoken(parser, lexer);
        parsexor(parser, lexer);
    } else if(checktoken(parser, TOK_OR)) {
        nexttoken(parser, lexer);
        parseor(parser, lexer);
    } else if(checktoken(parser, TOK_NOT)) {
        nexttoken(parser, lexer);
        parsenot(parser, lexer);
    } else if(checktoken(parser, TOK_NEG)) {
        nexttoken(parser, lexer);
        parseneg(parser, lexer);
    } else if(checktoken(parser, TOK_TEST)) {
        nexttoken(parser, lexer);
        parsetest(parser, lexer); 
    } else if(checktoken(parser, TOK_SYSCALL)) {
        nexttoken(parser, lexer);
        emitbyte(&emitter, 0x1F);
        emitbyte(&emitter, 0x00);
    } else if(checktoken(parser, TOK_HASHTAG)) { // MACRO
        nexttoken(parser, lexer);
        parsemacro(parser, lexer);
    } else if(checktoken(parser, TOK_PERIOD)) { // PERIOD OPERATOR
        nexttoken(parser, lexer);
        parseperiod(parser, lexer);
    } else if(checktoken(parser, TOK_LABEL)) {
        char *label = parser->curtoken.text;
        // printf("Label: '%s', %d\n", label, parser->curtoken.type);
        labelmapset(labels, label, emitter.written + emitter.offset);
        nexttoken(parser, lexer);
        match(parser, lexer, TOK_COLON);
        if(checktoken(parser, TOK_NEWLINE)) nexttoken(parser, lexer);
        return;
    }

    checknewline(parser, lexer);
}

void compilefile(char *buffer) {
    struct Parser comparser;

    // I don't know *how* this fixes it, it just does.
    struct Token token;
    strcpy(token.text, "\0");
    token.type = 0;
    comparser.curtoken = token;
    comparser.peektoken = token;

    struct Lexer lexer;
    initlexer(&lexer, buffer);

    while(checktoken(&comparser, TOK_NEWLINE)) nexttoken(&comparser, &lexer);

    while(!checktoken(&comparser, TOK_EOF)) { 
        parsestatement(&comparser, &lexer);
    }
}

int compiler(char *buffer, char *output, uint32_t offset, uint32_t basesize) {

    job.outputfile = output;
    
    initemitter(&emitter, output);
    emitter.offset = offset;
    emitter.basesize = basesize;
    labels = labelmapcreate();
    
    compilefile(buffer);
    
    for(size_t i = 0; i < relocatablepointer; i++) {
        uint32_t loc = relocatables[i]->loc;
        labelmapent_t *label = labelmapget(labels, relocatables[i]->symbol); // label with relevant symbol/
        if(label != NULL) {
            uint32_t labelloc = label->value;
            switch(relocatables[i]->mode) {
                case 0x01: // 8 bit
                    relocatebyte(&emitter, loc, labelloc);
                    break;
                case 0x02: // 16 bit
                    relocatebyte16(&emitter, loc, labelloc);
                    break;
                case 0x03: // 32 bit
                    // printf("relocating %s to 0x%08x at 0x%08x...\n", relocatables[i]->symbol, label->value, loc);
                    relocatebyte32(&emitter, loc, labelloc);
                    break;
            }
        } else {
            char *err = malloc(32);
            sprintf(err, "Undefined reference to symbol %s", relocatables[i]->symbol);
            parserabort(err);
            continue;
        }
    }

    writefile(&emitter);

    return 0;
}
