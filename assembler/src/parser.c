#include <string.h>
#include <stdint.h>
#include <stdio.h>

#include "common.h"
#include "compiler.h"
#include "emitter.h"
#include "lexer.h"

#define DEF_REG(_n, _dat) \
    else if(strcmp(text, #_n) == 0) { \
        return _dat; \
    }

uint8_t resolveregister(char *text) {
    if(strcmp(text, "ax") == 0) {
        return 0x00;
    }
    DEF_REG(bx, 0x01)
    DEF_REG(cx, 0x02)
    DEF_REG(dx, 0x03)
    DEF_REG(si, 0x04)
    DEF_REG(di, 0x05)
    DEF_REG(sp, 0x06)
    DEF_REG(bp, 0x07)
    DEF_REG(ip, 0x08)
    DEF_REG(r0, 0x09)
    DEF_REG(r1, 0x0A)
    DEF_REG(r2, 0x0B)
    DEF_REG(r3, 0x0C)
    DEF_REG(r4, 0x0D)
    DEF_REG(r5, 0x0E)
    DEF_REG(r6, 0x0F)
    DEF_REG(r7, 0x10)
    DEF_REG(r8, 0x11)
    DEF_REG(r9, 0x12)
    DEF_REG(r10, 0x13)
    DEF_REG(r11, 0x14)
    DEF_REG(r12, 0x15)
    DEF_REG(r13, 0x16)
    DEF_REG(r14, 0x17)
    DEF_REG(r15, 0x18)

    return 0x00; // NULL (Should not be reached)
}

char *resolvetoken(int type) {
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

int checktoken(struct Parser *parser, int type) {
    return type == parser->curtoken.type;
}

int checkpeek(struct Parser *parser, int type) {
    return type == parser->peektoken.type;
}

void parserabort(char *message) {
    printf("%s\n", message);
    exit(1);
}

void nexttoken(struct Parser *parser, struct Lexer *lexer) {
    parser->curtoken = parser->peektoken;
    parser->peektoken = gettoken(lexer);
}

void match(struct Parser *parser, struct Lexer *lexer, int type) {
    if(!checktoken(parser, type)) {
        char buf[128];
        sprintf(buf, "Expected %s got %s on line %lu", resolvetoken(type), resolvetoken(parser->curtoken.type), lexer->line);
        parserabort(buf);
    }
    nexttoken(parser, lexer);
}

void checknewline(struct Parser *parser, struct Lexer *lexer) {
    match(parser, lexer, TOK_NEWLINE);
    while(checktoken(parser, TOK_NEWLINE))
        nexttoken(parser, lexer);
}

void parsesymbol(struct Parser *parser, struct Lexer *lexer) {
    struct Token labeltok = parser->curtoken;
    nexttoken(parser, lexer); 
    labelmapent_t *label = labelmapget(labels, labeltok.text);
    if(!(label == NULL)) {
        emitbyte32(&emitter, label->value); // emit label
    } else {
        uint32_t current = emitter.written;
        emitbyte32(&emitter, 0x00000000); // emit zeros (replace later)
        struct Relocatable *relocatable = (struct Relocatable *)malloc(sizeof(struct Relocatable *)); // 32 bit relocatable located at this address.
        relocatable->symbol = strdup(labeltok.text);
        relocatable->loc = current;
        relocatable->mode = 0x03;
        relocatables[relocatablepointer++] = relocatable;
    }
}

void parsesymbolneg(struct Parser *parser, struct Lexer *lexer) {
    struct Token labeltok = parser->curtoken;
    nexttoken(parser, lexer); 
    labelmapent_t *label = labelmapget(labels, labeltok.text);
    if(!(label == NULL)) {
        emitbyte32(&emitter, -label->value); // emit label
    } else {
        uint32_t current = emitter.written;
        emitbyte32(&emitter, 0x00000000); // emit zeros (replace later)
        struct Relocatable *relocatable = (struct Relocatable *)malloc(sizeof(struct Relocatable *)); // 32 bit relocatable located at this address.
        relocatable->symbol = strdup(labeltok.text);
        relocatable->loc = current;
        relocatable->mode = 0x06; // negative 32-bit
        relocatables[relocatablepointer++] = relocatable;
    }
}

void parseptr(struct Parser *parser, struct Lexer *lexer) {
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
        } else if(checktoken(parser, TOK_LABEL)) {
            !isneg ? parsesymbol(parser, lexer) : parsesymbolneg(parser, lexer);
        }
    } else {
        emitbyte32(&emitter, 0x00000000); // null offset
    }
    match(parser, lexer, TOK_RSQUARE);
}
