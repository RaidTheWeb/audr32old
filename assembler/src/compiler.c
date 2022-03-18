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
    if(!label == NULL) {
        emitbyte32(&emitter, label->value); // emit label
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
        } else if(checktoken(parser, TOK_LABEL)) {
            parsesymbol(parser);
        }
    } else if(checktoken(parser, TOK_REGISTER)) {
        emitbyte(&emitter, 0x04); // REG
        emitbyte(&emitter, resolveregister(parser->curtoken.text));
        match(parser, TOK_REGISTER);
    } else if(checktoken(parser, TOK_LABEL)) {
        parsesymbol(parser);
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
            match(parser, TOK_NUMBER);
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
        match(parser, TOK_NUMBER);
    } else if(checktoken(parser, TOK_LABEL)) {
        emitbyte(&emitter, 0x02); // DAT
        parsesymbol(parser);
    }
}

static void parsejnz(struct Parser *parser) {
    // TODO: Implement JNZ >//<
    emitbyte(&emitter, 0x05); // JNZ
    
}

static void parsestatement(struct Parser *parser) {
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
        // TODO: parseint(parser);
    } else if(checktoken(parser, TOK_JMP)) {
        nexttoken(parser);
        parsejmp(parser);
    } else if(checktoken(parser, TOK_JNZ)) {
        nexttoken(parser);
        parsejnz(parser);
    } else if(checktoken(parser, TOK_JZ)) {
        nexttoken(parser);
        //parsejz(parser);
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

    printf("main labelmap: 0x%08x\n", labelmapget(labels, "main")->value);
    //emitbyte32(&emitter, 0x000000FF);

    writefile(&emitter);

    return 0;
}
