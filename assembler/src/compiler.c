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
struct Lexer lexer;
struct LabelMapPair *labels[SIZE];

struct Relocatable {
    char *symbol;
    uint32_t loc; // location where to relocate
    uint8_t mode; // 0x01, 0x02, 0x03 (8 bit, 16 bit, and 32 bit respectively)
};

struct Relocatable *relocatables[SIZE];
int relocatablepointer = 0;

struct Parser {
    struct Token curtoken;
    struct Token peektoken;
};
struct Parser comparser;

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
    DEF_REG(r8, 0x0A)
    DEF_REG(r9, 0x0B)
    DEF_REG(r10, 0x0C)
    DEF_REG(r11, 0x0D)
    DEF_REG(r12, 0x0E)
    DEF_REG(r13, 0x0F)
    DEF_REG(r14, 0x10)
    DEF_REG(r15, 0x11)

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
        case TOK_DEC:               return "DEC";
        case TOK_DIV:               return "DIV";
        case TOK_EOF:               return "EOF";
        case TOK_HALT:              return "HALT";
        case TOK_HASHTAG:           return "HASHTAG";
        case TOK_IADD:              return "IADD";
        case TOK_IDIV:              return "IDIV";
        case TOK_IMUL:              return "IMUL";
        case TOK_INC:               return "INC";
        case TOK_INT:               return "INT";
        case TOK_INX:               return "INX";
        case TOK_ISUB:              return "ISUB";
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
        case TOK_NOOP:              return "NOOP";
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
    }
    return "NOOP"; // Default to NOOP
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

static void nexttoken(struct Parser *parser) {
    parser->curtoken = parser->peektoken;
    parser->peektoken = gettoken(&lexer);
}

static void match(struct Parser *parser, int type) {
    if(!checktoken(parser, type)) {
        char buf[128];
        sprintf(buf, "Expected %s got %s", resolvetoken(type), resolvetoken(parser->curtoken.type));
        parserabort(buf);
    }
    nexttoken(parser);
}

static void checknewline(struct Parser *parser) {
    match(parser, TOK_NEWLINE);
    while(checktoken(parser, TOK_NEWLINE))
        nexttoken(parser);
}

static void parsesymbol(struct Parser *parser) {
    struct Token labeltok = parser->curtoken;
    printf("reference to %s\n", labeltok.text);
    nexttoken(parser); 
    struct LabelMapPair *label = labelmapget(labels, labeltok.text);
    if(!(label == NULL)) {
        emitbyte32(&emitter, label->value); // emit label
    } else {
        printf("reference to a label that doesn't exist.\n");
        uint32_t current = emitter.written;
        emitbyte32(&emitter, 0x00000000); // emit zeros (replace later)
        struct Relocatable *relocatable = (struct Relocatable *)malloc(sizeof(struct Relocatable *)); // 32 bit relocatable located at this address.
        relocatable->symbol = (char *)malloc(sizeof(labeltok.text) + 1);
        strcpy(relocatable->symbol, labeltok.text);
        relocatable->loc = current;
        relocatable->mode = 0x03;
        relocatables[relocatablepointer++] = relocatable;
    }
}

static void parseptr(struct Parser *parser) {
    nexttoken(parser);
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
        match(parser, TOK_NUMBER);
        match(parser, TOK_COLON);
        if(checktoken(parser, TOK_NUMBER)) {
            emitbyte(&emitter, mode);
            emitbyte32(&emitter, strtol(parser->curtoken.text, NULL, 10));
            match(parser, TOK_NUMBER);
        } else if(checktoken(parser, TOK_REGISTER)) {
            emitbyte(&emitter, 0x04 + (mode - 1));
            emitbyte(&emitter, resolveregister(parser->curtoken.text));
            match(parser, TOK_REGISTER);
        } else if(checktoken(parser, TOK_LABEL)) {
            emitbyte(&emitter, mode);
            parsesymbol(parser);
        }
    } else if(checktoken(parser, TOK_REGISTER)) {
        emitbyte(&emitter, 0x06); // REG32
        emitbyte(&emitter, resolveregister(parser->curtoken.text));
        match(parser, TOK_REGISTER);
    } else if(checktoken(parser, TOK_LABEL)) {
        emitbyte(&emitter, 0x03); // uh ok
        parsesymbol(parser);
    }
    if(checktoken(parser, TOK_COLON)) { // offset
        match(parser, TOK_COLON);
        int isneg = 0; // positive
        if(checktoken(parser, TOK_MINUS)) {
            isneg = 1; // negative
            match(parser, TOK_MINUS);
        }
        if(checktoken(parser, TOK_NUMBER)) {
            if(!isneg) emitbyte32(&emitter, strtol(parser->curtoken.text, NULL, 10)); 
            else emitbyte32(&emitter, -strtol(parser->curtoken.text, NULL, 10));
            match(parser, TOK_NUMBER);
        }
    } else {
        emitbyte32(&emitter, 0x00000000); // null offset
    }
    match(parser, TOK_RSQUARE);
}

static void parsemov(struct Parser *parser) {
    emitbyte(&emitter, 0x02); // MOV
    if(checktoken(parser, TOK_REGISTER)) {
        uint8_t reg = resolveregister(parser->curtoken.text);
        nexttoken(parser);
        match(parser, TOK_COMMA);
        if(checktoken(parser, TOK_NUMBER)) {
            emitbyte(&emitter, 0x02); // REGDAT
            emitbyte(&emitter, reg);
            emitbyte32(&emitter, strtol(parser->curtoken.text, NULL, 10));
            match(parser, TOK_NUMBER);
        } else if(checktoken(parser, TOK_CHAR)) {
            emitbyte(&emitter, 0x02); // REGDAT
            emitbyte(&emitter, reg);
            emitbyte32(&emitter, (char)parser->curtoken.text[0]);
            match(parser, TOK_CHAR);
        } else if(checktoken(parser, TOK_REGISTER)) {
            emitbyte(&emitter, 0x00); // REGREG
            emitbyte(&emitter, reg);
            emitbyte(&emitter, resolveregister(parser->curtoken.text));
            match(parser, TOK_REGISTER);
        } else if(checktoken(parser, TOK_LSQUARE)) { // possibly a pointer
            emitbyte(&emitter, 0x01); // REGPTR
            emitbyte(&emitter, reg);
            parseptr(parser);
        } else if(checktoken(parser, TOK_LABEL)) {
            emitbyte(&emitter, 0x02); // REGDAT
            emitbyte(&emitter, reg);
            parsesymbol(parser);
        }
    } else if(checktoken(parser, TOK_LSQUARE)) {
        uint32_t loc = emitter.written; // used when replacing the dummy mode with the real mode later
        emitbyte(&emitter, 0x00); // dummy mode (replaced later)
        parseptr(parser);
        match(parser, TOK_COMMA);
        if(checktoken(parser, TOK_REGISTER)) {
            relocatebyte(&emitter, loc, 0x03); // PTRREG
            uint8_t reg = resolveregister(parser->curtoken.text);
            emitbyte(&emitter, reg);
            match(parser, TOK_REGISTER);
        } else if(checktoken(parser, TOK_NUMBER)) {
            relocatebyte(&emitter, loc, 0x04); // PTRDAT
            emitbyte32(&emitter, strtol(parser->curtoken.text, NULL, 10));
            match(parser, TOK_NUMBER);
        } else if(checktoken(parser, TOK_CHAR)) {
            relocatebyte(&emitter, loc, 0x04); // PTRDAT
            emitbyte32(&emitter, (char)parser->curtoken.text[0]);
            match(parser, TOK_CHAR);
        } else if(checktoken(parser, TOK_LABEL)) {
            relocatebyte(&emitter, loc, 0x04); // PTRDAT
            parsesymbol(parser);
        } else if(checktoken(parser, TOK_LSQUARE)) {
            relocatebyte(&emitter, loc, 0x05); // PTRPTR
            parseptr(parser);
        }
    }
}

static void parsejmp(struct Parser *parser) {
    uint32_t start = emitter.written;
    emitbyte(&emitter, 0x04); // JMP
    if(checktoken(parser, TOK_REGISTER)) {
        emitbyte(&emitter, 0x00); // REG
        emitbyte(&emitter, resolveregister(parser->curtoken.text));
    } else if(checktoken(parser, TOK_LSQUARE)) {
        emitbyte(&emitter, 0x01); // PTR
        parseptr(parser);
    } else if(checktoken(parser, TOK_NUMBER)) {
        emitbyte(&emitter, 0x02); // DAT
        emitbyte32(&emitter, strtol(parser->curtoken.text, NULL, 10));
        match(parser, TOK_NUMBER);
    } else if(checktoken(parser, TOK_CHAR)) {
        emitbyte(&emitter, 0x02); // DAT
        emitbyte32(&emitter, (char)parser->curtoken.text[0]);
        match(parser, TOK_CHAR);
    } else if(checktoken(parser, TOK_LABEL)) {
        emitbyte(&emitter, 0x02); // DAT
        parsesymbol(parser);
    } else if(checktoken(parser, TOK_DOLLAR)) {
        emitbyte(&emitter, 0x02); // DAT
        emitbyte32(&emitter, start);
        match(parser, TOK_DOLLAR);
    }
}

static void parsejnz(struct Parser *parser) {
    uint32_t start = emitter.written;
    emitbyte(&emitter, 0x05); // JNZ

    // DEST
    if(checktoken(parser, TOK_NUMBER))  {
        emitbyte(&emitter, 0x02);
        emitbyte32(&emitter, strtol(parser->curtoken.text, NULL, 10));
        match(parser, TOK_NUMBER);
    } else if(checktoken(parser, TOK_CHAR)) {
        emitbyte(&emitter, 0x02); // DAT
        emitbyte32(&emitter, (char)parser->curtoken.text[0]);
        match(parser, TOK_CHAR);
    } else if(checktoken(parser, TOK_LABEL)) {
        emitbyte(&emitter, 0x02); // DAT
        parsesymbol(parser);
    } else if(checktoken(parser, TOK_REGISTER)) {
        emitbyte(&emitter, 0x00); // REG
        emitbyte(&emitter, resolveregister(parser->curtoken.text));
        match(parser, TOK_REGISTER);
    } else if(checktoken(parser, TOK_LSQUARE)) {
        emitbyte(&emitter, 0x01); // PTR
        parseptr(parser);
    } else if(checktoken(parser, TOK_DOLLAR)) {
        emitbyte(&emitter, 0x02); // DAT
        emitbyte32(&emitter, start);
        match(parser, TOK_DOLLAR);
    }
}

static void parsejz(struct Parser *parser) {
    uint32_t start = emitter.written;
    emitbyte(&emitter, 0x06); // JZ
    // DEST
    if(checktoken(parser, TOK_NUMBER))  {
        emitbyte(&emitter, 0x02);
        emitbyte32(&emitter, strtol(parser->curtoken.text, NULL, 10));
        match(parser, TOK_NUMBER);
    } else if(checktoken(parser, TOK_CHAR)) {
        emitbyte(&emitter, 0x02); // DAT
        emitbyte32(&emitter, (char)parser->curtoken.text[0]);
        match(parser, TOK_CHAR);
    } else if(checktoken(parser, TOK_LABEL)) {
        emitbyte(&emitter, 0x02); // DAT
        parsesymbol(parser);
    } else if(checktoken(parser, TOK_REGISTER)) {
        emitbyte(&emitter, 0x00); // REG
        emitbyte(&emitter, resolveregister(parser->curtoken.text));
        match(parser, TOK_REGISTER);
    } else if(checktoken(parser, TOK_LSQUARE)) {
        emitbyte(&emitter, 0x01); // PTR
        parseptr(parser);
    } else if(checktoken(parser, TOK_DOLLAR)) {
        emitbyte(&emitter, 0x02); // DAT
        emitbyte32(&emitter, start);
        match(parser, TOK_DOLLAR);
    }
}

static void parsejl(struct Parser *parser) {
    uint32_t start = emitter.written;
    emitbyte(&emitter, 0x1E); // JL
    // DEST
    if(checktoken(parser, TOK_NUMBER))  {
        emitbyte(&emitter, 0x02);
        emitbyte32(&emitter, strtol(parser->curtoken.text, NULL, 10));
        match(parser, TOK_NUMBER);
    } else if(checktoken(parser, TOK_CHAR)) {
        emitbyte(&emitter, 0x02); // DAT
        emitbyte32(&emitter, (char)parser->curtoken.text[0]);
        match(parser, TOK_CHAR);
    } else if(checktoken(parser, TOK_LABEL)) {
        emitbyte(&emitter, 0x02); // DAT
        parsesymbol(parser);
    } else if(checktoken(parser, TOK_REGISTER)) {
        emitbyte(&emitter, 0x00); // REG
        emitbyte(&emitter, resolveregister(parser->curtoken.text));
        match(parser, TOK_REGISTER);
    } else if(checktoken(parser, TOK_LSQUARE)) {
        emitbyte(&emitter, 0x01); // PTR
        parseptr(parser);
    }  else if(checktoken(parser, TOK_DOLLAR)) {
        emitbyte(&emitter, 0x02); // DAT
        emitbyte32(&emitter, start);
        match(parser, TOK_DOLLAR);
    }
}


static void parsejle(struct Parser *parser) {
    uint32_t start = emitter.written;
    emitbyte(&emitter, 0x1F); // JLE
    // DEST
    if(checktoken(parser, TOK_NUMBER))  {
        emitbyte(&emitter, 0x02);
        emitbyte32(&emitter, strtol(parser->curtoken.text, NULL, 10));
        match(parser, TOK_NUMBER);
    } else if(checktoken(parser, TOK_CHAR)) {
        emitbyte(&emitter, 0x02); // DAT
        emitbyte32(&emitter, (char)parser->curtoken.text[0]);
        match(parser, TOK_CHAR);
    } else if(checktoken(parser, TOK_LABEL)) {
        emitbyte(&emitter, 0x02); // DAT
        parsesymbol(parser);
    } else if(checktoken(parser, TOK_REGISTER)) {
        emitbyte(&emitter, 0x00); // REG
        emitbyte(&emitter, resolveregister(parser->curtoken.text));
        match(parser, TOK_REGISTER);
    } else if(checktoken(parser, TOK_LSQUARE)) {
        emitbyte(&emitter, 0x01); // PTR
        parseptr(parser);
    } else if(checktoken(parser, TOK_DOLLAR)) {
        emitbyte(&emitter, 0x02); // DAT
        emitbyte32(&emitter, start);
        match(parser, TOK_DOLLAR);
    }
}

static void parsejg(struct Parser *parser) {
    uint32_t start = emitter.written;
    emitbyte(&emitter, 0x20); // JG
    // DEST
    if(checktoken(parser, TOK_NUMBER))  {
        emitbyte(&emitter, 0x02);
        emitbyte32(&emitter, strtol(parser->curtoken.text, NULL, 10));
        match(parser, TOK_NUMBER);
    } else if(checktoken(parser, TOK_CHAR)) {
        emitbyte(&emitter, 0x02); // DAT
        emitbyte32(&emitter, (char)parser->curtoken.text[0]);
        match(parser, TOK_CHAR);
    } else if(checktoken(parser, TOK_LABEL)) {
        emitbyte(&emitter, 0x02); // DAT
        parsesymbol(parser);
    } else if(checktoken(parser, TOK_REGISTER)) {
        emitbyte(&emitter, 0x00); // REG
        emitbyte(&emitter, resolveregister(parser->curtoken.text));
        match(parser, TOK_REGISTER);
    } else if(checktoken(parser, TOK_LSQUARE)) {
        emitbyte(&emitter, 0x01); // PTR
        parseptr(parser);
    } else if(checktoken(parser, TOK_DOLLAR)) {
        emitbyte(&emitter, 0x02); // DAT
        emitbyte32(&emitter, start);
        match(parser, TOK_DOLLAR);
    }
}

static void parsejge(struct Parser *parser) {
    uint32_t start = emitter.written;
    emitbyte(&emitter, 0x21); // JGE
    // DEST
    if(checktoken(parser, TOK_NUMBER))  {
        emitbyte(&emitter, 0x02);
        emitbyte32(&emitter, strtol(parser->curtoken.text, NULL, 10));
        match(parser, TOK_NUMBER);
    } else if(checktoken(parser, TOK_CHAR)) {
        emitbyte(&emitter, 0x02); // DAT
        emitbyte32(&emitter, (char)parser->curtoken.text[0]);
        match(parser, TOK_CHAR);
    } else if(checktoken(parser, TOK_LABEL)) {
        emitbyte(&emitter, 0x02); // DAT
        parsesymbol(parser);
    } else if(checktoken(parser, TOK_REGISTER)) {
        emitbyte(&emitter, 0x00); // REG
        emitbyte(&emitter, resolveregister(parser->curtoken.text));
        match(parser, TOK_REGISTER);
    } else if(checktoken(parser, TOK_LSQUARE)) {
        emitbyte(&emitter, 0x01); // PTR
        parseptr(parser);
    } else if(checktoken(parser, TOK_DOLLAR)) {
        emitbyte(&emitter, 0x02); // DAT
        emitbyte32(&emitter, start);
        match(parser, TOK_DOLLAR);
    }

}


static void parsecmp(struct Parser *parser) {
    emitbyte(&emitter, 0x17); // CMP
    uint32_t loc = emitter.written;
    emitbyte(&emitter, 0x00); // mock mode
    uint8_t mode = 0x00; // default (REGREG)
    // DEST, SRC
    if(checktoken(parser, TOK_NUMBER))  {
        mode = 0x06; // DATXXX
        emitbyte32(&emitter, strtol(parser->curtoken.text, NULL, 10));
        match(parser, TOK_NUMBER);
    } else if(checktoken(parser, TOK_CHAR)) {
        mode = 0x06; // DATXXX
        emitbyte32(&emitter, (char)parser->curtoken.text[0]);
        match(parser, TOK_CHAR);
    } else if(checktoken(parser, TOK_LABEL)) {
        mode = 0x06; // DATXXX
        parsesymbol(parser);
    } else if(checktoken(parser, TOK_REGISTER)) {
        mode = 0x00; // REGXXX
        emitbyte(&emitter, resolveregister(parser->curtoken.text));
        match(parser, TOK_REGISTER);
    } else if(checktoken(parser, TOK_LSQUARE)) {
        mode = 0x03; // PTRXXX
        parseptr(parser);
    }

    match(parser, TOK_COMMA);

    if(checktoken(parser, TOK_NUMBER))  {
        mode += 2; // XXXDAT
        emitbyte32(&emitter, strtol(parser->curtoken.text, NULL, 10));
        match(parser, TOK_NUMBER);
    } else if(checktoken(parser, TOK_CHAR)) {
        mode += 2; // XXXDAT
        emitbyte32(&emitter, (char)parser->curtoken.text[0]);
        match(parser, TOK_CHAR);
    } else if(checktoken(parser, TOK_LABEL)) {
        mode += 2; // XXXDAT
        parsesymbol(parser);
    } else if(checktoken(parser, TOK_REGISTER)) {
        mode += 0; // XXXREG
        emitbyte(&emitter, resolveregister(parser->curtoken.text));
        match(parser, TOK_REGISTER);
    } else if(checktoken(parser, TOK_LSQUARE)) {
        mode += 1; // XXXPTR
        parseptr(parser);
    }

    relocatebyte(&emitter, loc, mode); // set mode
}

static void parseint(struct Parser *parser) {
    emitbyte(&emitter, 0x03); // INT
    if(checktoken(parser, TOK_NUMBER)) {
        emitbyte(&emitter, 0x00);
        emitbyte32(&emitter, strtol(parser->curtoken.text, NULL, 10));
        match(parser, TOK_NUMBER);
    } else if(checktoken(parser, TOK_CHAR)) {
        emitbyte(&emitter, 0x00);
        emitbyte32(&emitter, (char)parser->curtoken.text[0]);
        match(parser, TOK_CHAR);
    } else if(checktoken(parser, TOK_LABEL)) {
        emitbyte(&emitter, 0x00);
        parsesymbol(parser);
    } else if(checktoken(parser, TOK_REGISTER)) {
        emitbyte(&emitter, 0x01);
        emitbyte(&emitter, resolveregister(parser->curtoken.text));
        match(parser, TOK_REGISTER);
    } else if(checktoken(parser, TOK_LSQUARE)) {
        emitbyte(&emitter, 0x02);
        parseptr(parser);
    }
}

static void parsecall(struct Parser *parser) {
    emitbyte(&emitter, 0x07); // CALL
    if(checktoken(parser, TOK_NUMBER)) {
        emitbyte(&emitter, 0x00); // DEFAULT
        emitbyte32(&emitter, strtol(parser->curtoken.text, NULL, 10));
        match(parser, TOK_NUMBER);
    } else if(checktoken(parser, TOK_CHAR)) {
        emitbyte(&emitter, 0x00); // DEFAULT
        emitbyte32(&emitter, (char)parser->curtoken.text[0]);
        match(parser, TOK_CHAR);
    } else if(checktoken(parser, TOK_LABEL)) {
        emitbyte(&emitter, 0x00); // DEFAULT
        parsesymbol(parser);
    }
}

static void parseinx(struct Parser *parser) {
    emitbyte(&emitter, 0x09); // INX
    if(checktoken(parser, TOK_NUMBER)) {
        emitbyte(&emitter, 0x02); // DAT
        emitbyte32(&emitter, strtol(parser->curtoken.text, NULL, 10));
        match(parser, TOK_NUMBER);
    } else if(checktoken(parser, TOK_CHAR)) {
        emitbyte(&emitter, 0x02); // DAT
        emitbyte32(&emitter, (char)parser->curtoken.text[0]);
        match(parser, TOK_CHAR);
    } else if(checktoken(parser, TOK_LABEL)) {
        emitbyte(&emitter, 0x02); // DAT
        parsesymbol(parser);
    } else if(checktoken(parser, TOK_REGISTER)) {
        emitbyte(&emitter, 0x00); // REG
        emitbyte(&emitter, resolveregister(parser->curtoken.text));
        match(parser, TOK_REGISTER);
    } else if(checktoken(parser, TOK_LSQUARE)) {
        emitbyte(&emitter, 0x01); // PTR
        parseptr(parser);
    }
}

static void parseoutx(struct Parser *parser) {
    emitbyte(&emitter, 0x0A); // OUTX
    uint32_t loc = emitter.written;
    emitbyte(&emitter, 0x00); // mock mode
    uint8_t mode = 0x00; // default (REGREG)
    // DEST, SRC
    if(checktoken(parser, TOK_NUMBER))  {
        mode = 0x06; // DATXXX
        emitbyte32(&emitter, strtol(parser->curtoken.text, NULL, 10));
        match(parser, TOK_NUMBER);
    } else if(checktoken(parser, TOK_CHAR)) {
        mode = 0x06; // DATXXX
        emitbyte32(&emitter, (char)parser->curtoken.text[0]);
        match(parser, TOK_CHAR);
    } else if(checktoken(parser, TOK_LABEL)) {
        mode = 0x06; // DATXXX
        parsesymbol(parser);
    } else if(checktoken(parser, TOK_REGISTER)) {
        mode = 0x00; // REGXXX
        emitbyte(&emitter, resolveregister(parser->curtoken.text));
        match(parser, TOK_REGISTER);
    } else if(checktoken(parser, TOK_LSQUARE)) {
        mode = 0x03; // PTRXXX
        parseptr(parser);
    }

    match(parser, TOK_COMMA);

    if(checktoken(parser, TOK_NUMBER))  {
        mode += 2; // XXXDAT
        emitbyte32(&emitter, strtol(parser->curtoken.text, NULL, 10));
        match(parser, TOK_NUMBER);
    } else if(checktoken(parser, TOK_CHAR)) {
        mode += 2; // XXXDAT
        emitbyte32(&emitter, (char)parser->curtoken.text[0]);
        match(parser, TOK_CHAR);
    } else if(checktoken(parser, TOK_LABEL)) {
        mode += 2; // XXXDAT
        parsesymbol(parser);
    } else if(checktoken(parser, TOK_REGISTER)) {
        mode += 0; // XXXREG
        emitbyte(&emitter, resolveregister(parser->curtoken.text));
        match(parser, TOK_REGISTER);
    } else if(checktoken(parser, TOK_LSQUARE)) {
        mode += 1; // XXXPTR
        parseptr(parser);
    }

    relocatebyte(&emitter, loc, mode); // set mode
}

static void parsepop(struct Parser *parser) {
    emitbyte(&emitter, 0x0B); // POP
    if(checktoken(parser, TOK_REGISTER)) {
        emitbyte(&emitter, 0x00); // REG
        emitbyte(&emitter, resolveregister(parser->curtoken.text));
        match(parser, TOK_REGISTER);
    } else if(checktoken(parser, TOK_LSQUARE)) {
        emitbyte(&emitter, 0x01); // PTR
        parseptr(parser);
    }
}

static void parsepush(struct Parser *parser) {
    emitbyte(&emitter, 0x0C); // PUSH
    if(checktoken(parser, TOK_NUMBER)) {
        emitbyte(&emitter, 0x02); // DAT
        emitbyte32(&emitter, strtol(parser->curtoken.text, NULL, 10));
        match(parser, TOK_NUMBER);
    } else if(checktoken(parser, TOK_CHAR)) {
        emitbyte(&emitter, 0x02); // DAT
        emitbyte32(&emitter, (char)parser->curtoken.text[0]);
        match(parser, TOK_CHAR);
    } else if(checktoken(parser, TOK_LABEL)) {
        emitbyte(&emitter, 0x02); // DAT
        parsesymbol(parser);
    } else if(checktoken(parser, TOK_REGISTER)) {
        emitbyte(&emitter, 0x00); // REG
        emitbyte(&emitter, resolveregister(parser->curtoken.text));
        match(parser, TOK_REGISTER);
    } else if(checktoken(parser, TOK_LSQUARE)) {
        emitbyte(&emitter, 0x01); // PTR
        parseptr(parser);
    }

}

static void parseadd(struct Parser *parser) {
    emitbyte(&emitter, 0x0D); // ADD
    uint32_t loc = emitter.written;
    emitbyte(&emitter, 0x00); // mock mode
    uint8_t mode = 0x00; // default (REGREG)
    // DEST, SRC
    if(checktoken(parser, TOK_REGISTER)) {
        mode = 0x00; // REGXXX
        emitbyte(&emitter, resolveregister(parser->curtoken.text));
        match(parser, TOK_REGISTER);
    } else if(checktoken(parser, TOK_LSQUARE)) {
        mode = 0x03; // PTRXXX
        parseptr(parser);
    }

    match(parser, TOK_COMMA);

    if(checktoken(parser, TOK_NUMBER))  {
        mode += 2; // XXXDAT
        emitbyte32(&emitter, strtol(parser->curtoken.text, NULL, 10));
        match(parser, TOK_NUMBER);
    } else if(checktoken(parser, TOK_CHAR)) {
        mode += 2; // XXXDAT
        emitbyte32(&emitter, (char)parser->curtoken.text[0]);
        match(parser, TOK_CHAR);
    } else if(checktoken(parser, TOK_LABEL)) {
        mode += 2; // XXXDAT
        parsesymbol(parser);
    } else if(checktoken(parser, TOK_REGISTER)) {
        mode += 0; // XXXREG
        emitbyte(&emitter, resolveregister(parser->curtoken.text));
        match(parser, TOK_REGISTER);
    } else if(checktoken(parser, TOK_LSQUARE)) {
        mode += 1; // XXXPTR
        parseptr(parser);
    }

    relocatebyte(&emitter, loc, mode); // set mode
}

static void parsesub(struct Parser *parser) {
    emitbyte(&emitter, 0x0F); // SUB
    uint32_t loc = emitter.written;
    emitbyte(&emitter, 0x00); // mock mode
    uint8_t mode = 0x00; // default (REGREG)
    // DEST, SRC
    if(checktoken(parser, TOK_REGISTER)) {
        mode = 0x00; // REGXXX
        emitbyte(&emitter, resolveregister(parser->curtoken.text));
        match(parser, TOK_REGISTER);
    } else if(checktoken(parser, TOK_LSQUARE)) {
        mode = 0x03; // PTRXXX
        parseptr(parser);
    }

    match(parser, TOK_COMMA);

    if(checktoken(parser, TOK_NUMBER))  {
        mode += 2; // XXXDAT
        emitbyte32(&emitter, strtol(parser->curtoken.text, NULL, 10));
        match(parser, TOK_NUMBER);
    } else if(checktoken(parser, TOK_CHAR)) {
        mode += 2; // XXXDAT
        emitbyte32(&emitter, (char)parser->curtoken.text[0]);
        match(parser, TOK_CHAR);
    } else if(checktoken(parser, TOK_LABEL)) {
        mode += 2; // XXXDAT
        parsesymbol(parser);
    } else if(checktoken(parser, TOK_REGISTER)) {
        mode += 0; // XXXREG
        emitbyte(&emitter, resolveregister(parser->curtoken.text));
        match(parser, TOK_REGISTER);
    } else if(checktoken(parser, TOK_LSQUARE)) {
        mode += 1; // XXXPTR
        parseptr(parser);
    }

    relocatebyte(&emitter, loc, mode); // set mode
}

static void parsediv(struct Parser *parser) {
    emitbyte(&emitter, 0x11); // DIV
    uint32_t loc = emitter.written;
    emitbyte(&emitter, 0x00); // mock mode
    uint8_t mode = 0x00; // default (REGREG)
    // DEST, SRC
    if(checktoken(parser, TOK_REGISTER)) {
        mode = 0x00; // REGXXX
        emitbyte(&emitter, resolveregister(parser->curtoken.text));
        match(parser, TOK_REGISTER);
    } else if(checktoken(parser, TOK_LSQUARE)) {
        mode = 0x03; // PTRXXX
        parseptr(parser);
    }

    match(parser, TOK_COMMA);

    if(checktoken(parser, TOK_NUMBER))  {
        mode += 2; // XXXDAT
        emitbyte32(&emitter, strtol(parser->curtoken.text, NULL, 10));
        match(parser, TOK_NUMBER);
    } else if(checktoken(parser, TOK_CHAR)) {
        mode += 2; // XXXDAT
        emitbyte32(&emitter, (char)parser->curtoken.text[0]);
        match(parser, TOK_CHAR);
    } else if(checktoken(parser, TOK_LABEL)) {
        mode += 2; // XXXDAT
        parsesymbol(parser);
    } else if(checktoken(parser, TOK_REGISTER)) {
        mode += 0; // XXXREG
        emitbyte(&emitter, resolveregister(parser->curtoken.text));
        match(parser, TOK_REGISTER);
    } else if(checktoken(parser, TOK_LSQUARE)) {
        mode += 1; // XXXPTR
        parseptr(parser);
    }

    relocatebyte(&emitter, loc, mode); // set mode
}

static void parsemul(struct Parser *parser) {
    emitbyte(&emitter, 0x13); // MUL
    uint32_t loc = emitter.written;
    emitbyte(&emitter, 0x00); // mock mode
    uint8_t mode = 0x00; // default (REGREG)
    // DEST, SRC
    if(checktoken(parser, TOK_REGISTER)) {
        mode = 0x00; // REGXXX
        emitbyte(&emitter, resolveregister(parser->curtoken.text));
        match(parser, TOK_REGISTER);
    } else if(checktoken(parser, TOK_LSQUARE)) {
        mode = 0x03; // PTRXXX
        parseptr(parser);
    }

    match(parser, TOK_COMMA);

    if(checktoken(parser, TOK_NUMBER))  {
        mode += 2; // XXXDAT
        emitbyte32(&emitter, strtol(parser->curtoken.text, NULL, 10));
        match(parser, TOK_NUMBER);
    } else if(checktoken(parser, TOK_CHAR)) {
        mode += 2; // XXXDAT
        emitbyte32(&emitter, (char)parser->curtoken.text[0]);
        match(parser, TOK_CHAR);
    } else if(checktoken(parser, TOK_LABEL)) {
        mode += 2; // XXXDAT
        parsesymbol(parser);
    } else if(checktoken(parser, TOK_REGISTER)) {
        mode += 0; // XXXREG
        emitbyte(&emitter, resolveregister(parser->curtoken.text));
        match(parser, TOK_REGISTER);
    } else if(checktoken(parser, TOK_LSQUARE)) {
        mode += 1; // XXXPTR
        parseptr(parser);
    }

    relocatebyte(&emitter, loc, mode); // set mode
}

static void parseinc(struct Parser *parser) {
    emitbyte(&emitter, 0x15); // INC
    uint32_t loc = emitter.written;
    emitbyte(&emitter, 0x00); // mock mode
    uint8_t mode = 0x00; // default (REGREG)
    // DEST, SRC
    if(checktoken(parser, TOK_REGISTER)) {
        mode = 0x00; // REGXXX
        emitbyte(&emitter, resolveregister(parser->curtoken.text));
        match(parser, TOK_REGISTER);
    } else if(checktoken(parser, TOK_LSQUARE)) {
        mode = 0x01; // PTRXXX
        parseptr(parser);
    }

    relocatebyte(&emitter, loc, mode); // set mode
}

static void parsedec(struct Parser *parser) {
    emitbyte(&emitter, 0x16); // DEC
    uint32_t loc = emitter.written;
    emitbyte(&emitter, 0x00); // mock mode
    uint8_t mode = 0x00; // default (REGREG)
    // DEST, SRC
    if(checktoken(parser, TOK_REGISTER)) {
        mode = 0x00; // REGXXX
        emitbyte(&emitter, resolveregister(parser->curtoken.text));
        match(parser, TOK_REGISTER);
    } else if(checktoken(parser, TOK_LSQUARE)) {
        mode = 0x01; // PTRXXX
        parseptr(parser);
    }

    relocatebyte(&emitter, loc, mode); // set mode
}

static void parseand(struct Parser *parser) {
    emitbyte(&emitter, 0x18); // AND
    uint32_t loc = emitter.written;
    emitbyte(&emitter, 0x00); // mock mode
    uint8_t mode = 0x00; // default (REGREG)
    // DEST, SRC
    if(checktoken(parser, TOK_REGISTER)) {
        mode = 0x00; // REGXXX
        emitbyte(&emitter, resolveregister(parser->curtoken.text));
        match(parser, TOK_REGISTER);
    } else if(checktoken(parser, TOK_LSQUARE)) {
        mode = 0x03; // PTRXXX
        parseptr(parser);
    }

    match(parser, TOK_COMMA);

    if(checktoken(parser, TOK_NUMBER))  {
        mode += 2; // XXXDAT
        emitbyte32(&emitter, strtol(parser->curtoken.text, NULL, 10));
        match(parser, TOK_NUMBER);
    } else if(checktoken(parser, TOK_CHAR)) {
        mode += 2; // XXXDAT
        emitbyte32(&emitter, (char)parser->curtoken.text[0]);
        match(parser, TOK_CHAR);
    } else if(checktoken(parser, TOK_LABEL)) {
        mode += 2; // XXXDAT
        parsesymbol(parser);
    } else if(checktoken(parser, TOK_REGISTER)) {
        mode += 0; // XXXREG
        emitbyte(&emitter, resolveregister(parser->curtoken.text));
        match(parser, TOK_REGISTER);
    } else if(checktoken(parser, TOK_LSQUARE)) {
        mode += 1; // XXXPTR
        parseptr(parser);
    }

    relocatebyte(&emitter, loc, mode); // set mode
}

static void parseshl(struct Parser *parser) {
    emitbyte(&emitter, 0x19); // SHL
    uint32_t loc = emitter.written;
    emitbyte(&emitter, 0x00); // mock mode
    uint8_t mode = 0x00; // default (REGREG)
    // DEST, SRC
    if(checktoken(parser, TOK_REGISTER)) {
        mode = 0x00; // REGXXX
        emitbyte(&emitter, resolveregister(parser->curtoken.text));
        match(parser, TOK_REGISTER);
    } else if(checktoken(parser, TOK_LSQUARE)) {
        mode = 0x03; // PTRXXX
        parseptr(parser);
    }

    match(parser, TOK_COMMA);

    if(checktoken(parser, TOK_NUMBER))  {
        mode += 2; // XXXDAT
        emitbyte32(&emitter, strtol(parser->curtoken.text, NULL, 10));
        match(parser, TOK_NUMBER);
    } else if(checktoken(parser, TOK_CHAR)) {
        mode += 2; // XXXDAT
        emitbyte32(&emitter, (char)parser->curtoken.text[0]);
        match(parser, TOK_CHAR);
    } else if(checktoken(parser, TOK_LABEL)) {
        mode += 2; // XXXDAT
        parsesymbol(parser);
    } else if(checktoken(parser, TOK_REGISTER)) {
        mode += 0; // XXXREG
        emitbyte(&emitter, resolveregister(parser->curtoken.text));
        match(parser, TOK_REGISTER);
    } else if(checktoken(parser, TOK_LSQUARE)) {
        mode += 1; // XXXPTR
        parseptr(parser);
    }

    relocatebyte(&emitter, loc, mode); // set mode
}

static void parseshr(struct Parser *parser) {
    emitbyte(&emitter, 0x1A); // SHR
    uint32_t loc = emitter.written;
    emitbyte(&emitter, 0x00); // mock mode
    uint8_t mode = 0x00; // default (REGREG)
    // DEST, SRC
    if(checktoken(parser, TOK_REGISTER)) {
        mode = 0x00; // REGXXX
        emitbyte(&emitter, resolveregister(parser->curtoken.text));
        match(parser, TOK_REGISTER);
    } else if(checktoken(parser, TOK_LSQUARE)) {
        mode = 0x03; // PTRXXX
        parseptr(parser);
    }

    match(parser, TOK_COMMA);

    if(checktoken(parser, TOK_NUMBER))  {
        mode += 2; // XXXDAT
        emitbyte32(&emitter, strtol(parser->curtoken.text, NULL, 10));
        match(parser, TOK_NUMBER);
    } else if(checktoken(parser, TOK_CHAR)) {
        mode += 2; // XXXDAT
        emitbyte32(&emitter, (char)parser->curtoken.text[0]);
        match(parser, TOK_CHAR);
    } else if(checktoken(parser, TOK_LABEL)) {
        mode += 2; // XXXDAT
        parsesymbol(parser);
    } else if(checktoken(parser, TOK_REGISTER)) {
        mode += 0; // XXXREG
        emitbyte(&emitter, resolveregister(parser->curtoken.text));
        match(parser, TOK_REGISTER);
    } else if(checktoken(parser, TOK_LSQUARE)) {
        mode += 1; // XXXPTR
        parseptr(parser);
    }

    relocatebyte(&emitter, loc, mode); // set mode
}

static void parsexor(struct Parser *parser) {
    emitbyte(&emitter, 0x1B); // XOR
    uint32_t loc = emitter.written;
    emitbyte(&emitter, 0x00); // mock mode
    uint8_t mode = 0x00; // default (REGREG)
    // DEST, SRC
    if(checktoken(parser, TOK_REGISTER)) {
        mode = 0x00; // REGXXX
        emitbyte(&emitter, resolveregister(parser->curtoken.text));
        match(parser, TOK_REGISTER);
    } else if(checktoken(parser, TOK_LSQUARE)) {
        mode = 0x03; // PTRXXX
        parseptr(parser);
    }

    match(parser, TOK_COMMA);

    if(checktoken(parser, TOK_NUMBER))  {
        mode += 2; // XXXDAT
        emitbyte32(&emitter, strtol(parser->curtoken.text, NULL, 10));
        match(parser, TOK_NUMBER);
    } else if(checktoken(parser, TOK_CHAR)) {
        mode += 2; // XXXDAT
        emitbyte32(&emitter, (char)parser->curtoken.text[0]);
        match(parser, TOK_CHAR);
    } else if(checktoken(parser, TOK_LABEL)) {
        mode += 2; // XXXDAT
        parsesymbol(parser);
    } else if(checktoken(parser, TOK_REGISTER)) {
        mode += 0; // XXXREG
        emitbyte(&emitter, resolveregister(parser->curtoken.text));
        match(parser, TOK_REGISTER);
    } else if(checktoken(parser, TOK_LSQUARE)) {
        mode += 1; // XXXPTR
        parseptr(parser);
    }

    relocatebyte(&emitter, loc, mode); // set mode
}

static void parseor(struct Parser *parser) {
    emitbyte(&emitter, 0x1C); // OR
    uint32_t loc = emitter.written;
    emitbyte(&emitter, 0x00); // mock mode
    uint8_t mode = 0x00; // default (REGREG)
    // DEST, SRC
    if(checktoken(parser, TOK_REGISTER)) {
        mode = 0x00; // REGXXX
        emitbyte(&emitter, resolveregister(parser->curtoken.text));
        match(parser, TOK_REGISTER);
    } else if(checktoken(parser, TOK_LSQUARE)) {
        mode = 0x03; // PTRXXX
        parseptr(parser);
    }

    match(parser, TOK_COMMA);

    if(checktoken(parser, TOK_NUMBER))  {
        mode += 2; // XXXDAT
        emitbyte32(&emitter, strtol(parser->curtoken.text, NULL, 10));
        match(parser, TOK_NUMBER);
    } else if(checktoken(parser, TOK_CHAR)) {
        mode += 2; // XXXDAT
        emitbyte32(&emitter, (char)parser->curtoken.text[0]);
        match(parser, TOK_CHAR);
    } else if(checktoken(parser, TOK_LABEL)) {
        mode += 2; // XXXDAT
        parsesymbol(parser);
    } else if(checktoken(parser, TOK_REGISTER)) {
        mode += 0; // XXXREG
        emitbyte(&emitter, resolveregister(parser->curtoken.text));
        match(parser, TOK_REGISTER);
    } else if(checktoken(parser, TOK_LSQUARE)) {
        mode += 1; // XXXPTR
        parseptr(parser);
    }

    relocatebyte(&emitter, loc, mode); // set mode
}

static void parsenot(struct Parser *parser) {
    emitbyte(&emitter, 0x1D); // NOT
    uint32_t loc = emitter.written;
    emitbyte(&emitter, 0x00); // mock mode
    uint8_t mode = 0x00; // default (REGREG)
    // DEST, SRC
    if(checktoken(parser, TOK_REGISTER)) {
        mode = 0x00; // REGXXX
        emitbyte(&emitter, resolveregister(parser->curtoken.text));
        match(parser, TOK_REGISTER);
    } else if(checktoken(parser, TOK_LSQUARE)) {
        mode = 0x01; // PTRXXX
        parseptr(parser);
    }

    relocatebyte(&emitter, loc, mode); // set mode
}

// section
static int mode = 0; // text (0 text, 1 data)


// ---------
//  MACRO
// ---------

static void parsemacro(struct Parser *parser) {
    struct Token macroname = parser->curtoken;
    match(parser, TOK_LABEL); // expect macro operator
    if(strcmp(macroname.text, "define") == 0) { // constant
        struct Token constant = parser->curtoken;
        match(parser, TOK_LABEL);
        if(checktoken(parser, TOK_NUMBER)) {
            labelmapput(labels, constant.text, strtol(parser->curtoken.text, NULL, 10));
            match(parser, TOK_NUMBER);
        } else if(checktoken(parser, TOK_CHAR)) {
            labelmapput(labels, constant.text, (char)parser->curtoken.text[0]);
            match(parser, TOK_CHAR);
        }
    } else if(strcmp(macroname.text, "org") == 0) { // data origin
        if(checktoken(parser, TOK_NUMBER)) {
            emitter.cp = strtol(parser->curtoken.text, NULL, 10);
            emitter.written = strtol(parser->curtoken.text, NULL, 10);
            match(parser, TOK_NUMBER);
        }
    } else if((strcmp(macroname.text, "incbin") == 0) && mode == 1) { // include a binary
        struct Token constant = parser->curtoken;
        match(parser, TOK_STRING);
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

static void parseperiod(struct Parser *parser) {
    struct Token operator = parser->curtoken;
    match(parser, TOK_LABEL);
    if(strcmp(operator.text, "text") == 0) { // code
        mode = 0;
        return;
    } else if(strcmp(operator.text, "data") == 0) { // raw data
        printf(".data section\n");
        mode = 1;
        return;
    }

    if(mode == 0) {
        // nothing to do
    } else {
    
        if(strcmp(operator.text, "byte") == 0) { // 8 bit integer
            if(checktoken(parser, TOK_NUMBER)) {
                emitbyte(&emitter, strtol(parser->curtoken.text, NULL, 10));
                match(parser, TOK_NUMBER);
            } else if(checktoken(parser, TOK_CHAR)) {
                emitbyte(&emitter, (char)parser->curtoken.text[0]);
                match(parser, TOK_CHAR);
            } else if(checktoken(parser, TOK_LABEL)) {
                parsesymbol(parser);
            }
        } else if(strcmp(operator.text, "word") == 0) { // 16 bit integer
            if(checktoken(parser, TOK_NUMBER)) {
                emitbyte16(&emitter, strtol(parser->curtoken.text, NULL, 10));
                match(parser, TOK_NUMBER);
            } else if(checktoken(parser, TOK_CHAR)) {
                emitbyte16(&emitter, (char)parser->curtoken.text[0]);
                match(parser, TOK_CHAR);
            } else if(checktoken(parser, TOK_LABEL)) {
                parsesymbol(parser);
            }
        } else if(strcmp(operator.text, "dword") == 0) { // 32 bit integer
            if(checktoken(parser, TOK_NUMBER)) {
                emitbyte32(&emitter, strtol(parser->curtoken.text, NULL, 10));
                match(parser, TOK_NUMBER);
            } else if(checktoken(parser, TOK_CHAR)) {
                emitbyte32(&emitter, (char)parser->curtoken.text[0]);
                match(parser, TOK_CHAR);
            } else if(checktoken(parser, TOK_LABEL)) {
                parsesymbol(parser);
            }
        } else if((strcmp(operator.text, "asciiz") == 0) || (strcmp(operator.text, "string") == 0)) { // NULL terminated string
            struct Token data = parser->curtoken;
            printf("parsing .asciiz/.string\n");
            match(parser, TOK_STRING);
            for(size_t i = 0; i < strlen(data.text); i++) {
                emitbyte(&emitter, data.text[i]);
            }
            emitbyte(&emitter, 0x00); // null terminate
        } else if(strcmp(operator.text, "ascii") == 0) { // string (Not NULL terminated)
            struct Token data = parser->curtoken;
            match(parser, TOK_STRING);
            for(size_t i = 0; i < strlen(data.text); i++) {
                emitbyte(&emitter, data.text[i]);
            }
        }

    }
}

static void parsestatement(struct Parser *parser) {
    printf("Parsing/Assembling in mode 0x%02x\n", mode);
    if(mode == 1) { // data
        if(checktoken(parser, TOK_PERIOD)) { // actual operations
            printf("period operation.\n");
            nexttoken(parser);
            parseperiod(parser);
        } else if(checktoken(parser, TOK_HASHTAG)) { // MACRO
            nexttoken(parser);
            parsemacro(parser);
        } else if(checktoken(parser, TOK_LABEL)) {
            char *label = parser->curtoken.text;
            printf("Label: '%s', %d\n", label, parser->curtoken.type);
            labelmapput(labels, label, emitter.written);
            nexttoken(parser);
            match(parser, TOK_COLON);
        }

        checknewline(parser);
        return;
    }

    // effective ELSE
    if(checktoken(parser, TOK_NOOP)) {
        nexttoken(parser);
        emitbyte(&emitter, 0x00);
        emitbyte(&emitter, 0x00);
    } else if(checktoken(parser, TOK_HALT)) {
        nexttoken(parser);
        emitbyte(&emitter, 0x01);
        emitbyte(&emitter, 0x00);
    } else if(checktoken(parser, TOK_MOV)) {
        nexttoken(parser);
        parsemov(parser);
    } else if(checktoken(parser, TOK_INT)) {
        nexttoken(parser);
        parseint(parser);
    } else if(checktoken(parser, TOK_JMP)) {
        nexttoken(parser);
        parsejmp(parser);
    } else if(checktoken(parser, TOK_JNZ)) {
        nexttoken(parser);
        parsejnz(parser);
    } else if(checktoken(parser, TOK_JZ)) {
        nexttoken(parser);
        parsejz(parser);
    } else if(checktoken(parser, TOK_JNE)) {
        nexttoken(parser);
        parsejnz(parser);
    } else if(checktoken(parser, TOK_JE)) {
        nexttoken(parser);
        parsejz(parser);
    } else if(checktoken(parser, TOK_JL)) {
        nexttoken(parser);
        parsejl(parser);
    } else if(checktoken(parser, TOK_JLE)) {
        nexttoken(parser);
        parsejle(parser);
    } else if(checktoken(parser, TOK_JG)) {
        nexttoken(parser);
        parsejg(parser);
    } else if(checktoken(parser, TOK_JGE)) {
        nexttoken(parser);
        parsejge(parser);
    } else if(checktoken(parser, TOK_CMP)) {
        nexttoken(parser);
        parsecmp(parser);
    } else if(checktoken(parser, TOK_CALL)) {
        nexttoken(parser);
        parsecall(parser);
    } else if(checktoken(parser, TOK_RET)) {
        nexttoken(parser);
        emitbyte(&emitter, 0x08); // RET
        emitbyte(&emitter, 0x00);
    } else if(checktoken(parser, TOK_INX)) {
        nexttoken(parser);
        parseinx(parser);
    } else if(checktoken(parser, TOK_OUTX)) {
        nexttoken(parser);
        parseoutx(parser);
    } else if(checktoken(parser, TOK_POP)) {
        nexttoken(parser);
        parsepop(parser);
    } else if(checktoken(parser, TOK_PUSH)) {
        nexttoken(parser);
        parsepush(parser);
    } else if(checktoken(parser, TOK_ADD)) {
        nexttoken(parser);
        parseadd(parser);
    } else if(checktoken(parser, TOK_SUB)) {
        nexttoken(parser);
        parsesub(parser);
    } else if(checktoken(parser, TOK_DIV)) {
        nexttoken(parser);
        parsediv(parser);
    } else if(checktoken(parser, TOK_MUL)) {
        nexttoken(parser);
        parsemul(parser);
    } else if(checktoken(parser, TOK_INC)) {
        nexttoken(parser);
        parseinc(parser);
    } else if(checktoken(parser, TOK_DEC)) {
        nexttoken(parser);
        parsedec(parser);
    } else if(checktoken(parser, TOK_AND)) {
        nexttoken(parser);
        parseand(parser);
    } else if(checktoken(parser, TOK_SHL)) {
        nexttoken(parser);
        parseshl(parser);
    } else if(checktoken(parser, TOK_SHR)) {
        nexttoken(parser);
        parseshr(parser);
    } else if(checktoken(parser, TOK_XOR)) {
        nexttoken(parser);
        parsexor(parser);
    } else if(checktoken(parser, TOK_OR)) {
        nexttoken(parser);
        parseor(parser);
    } else if(checktoken(parser, TOK_NOT)) {
        nexttoken(parser);
        parsenot(parser);
    } else if(checktoken(parser, TOK_HASHTAG)) { // MACRO
        nexttoken(parser);
        parsemacro(parser);
    } else if(checktoken(parser, TOK_PERIOD)) { // PERIOD OPERATOR
        nexttoken(parser);
        parseperiod(parser);
    } else if(checktoken(parser, TOK_LABEL)) {
        char *label = parser->curtoken.text;
        printf("Label: '%s', %d\n", label, parser->curtoken.type);
        labelmapput(labels, label, emitter.written);
        nexttoken(parser);
        match(parser, TOK_COLON);
    }
    checknewline(parser);
}


int compiler(char *buffer, char *output) {

    job.outputfile = output;
    initlexer(&lexer, buffer);
    initemitter(&emitter, output);
    
    while(checktoken(&comparser, TOK_NEWLINE)) nexttoken(&comparser);

    while(!checktoken(&comparser, TOK_EOF)) {
        parsestatement(&comparser);
    }

    //printf("main labelmap: 0x%08x\n", labelmapget(labels, "main")->value);
    //emitbyte32(&emitter, 0x000000FF);
    
    for(size_t i = 0; i < relocatablepointer; i++) {
        uint32_t loc = relocatables[i]->loc;
        struct LabelMapPair *label = labelmapget(labels, relocatables[i]->symbol); // label with relevant symbol/
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
                    printf("relocating %s to 0x%08x at 0x%08x...\n", relocatables[i]->symbol, label->value, loc);
                    relocatebyte32(&emitter, loc, labelloc);
                    break;
            }
        } else {
            continue;
        }
    }

    writefile(&emitter);

    return 0;
}
