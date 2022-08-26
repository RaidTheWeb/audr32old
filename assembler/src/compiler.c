#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdint.h>

#include "common.h"
#include "compiler.h"
#include "datamap.h"
#include "directives.h"
#include "emitter.h"
#include "lexer.h"
#include "parser.h"


/*********************************************************************
 * Audr32 Assembler
 * compiler.c - Compile down to flat binary/mf executable.
 *
 * Do everything needed in here. >//<
 * Assembler specification is in `../ASM.txt`
 *********************************************************************/

struct Emitter emitter;
labelmap_t *labels; 

struct Relocatable *relocatables[INITIAL_CAPACITY];
int relocatablepointer = 0;

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
            relocatebyte(&emitter, loc, 0x05); // PTRDAT
            emitbyte32(&emitter, strtol(parser->curtoken.text, NULL, 10));
            match(parser, lexer, TOK_NUMBER);
        } else if(checktoken(parser, TOK_CHAR)) {
            relocatebyte(&emitter, loc, 0x05); // PTRDAT
            emitbyte32(&emitter, (char)parser->curtoken.text[0]);
            match(parser, lexer, TOK_CHAR);
        } else if(checktoken(parser, TOK_LABEL)) {
            relocatebyte(&emitter, loc, 0x05); // PTRDAT
            parsesymbol(parser, lexer);
        } else if(checktoken(parser, TOK_LSQUARE)) {
            relocatebyte(&emitter, loc, 0x04); // PTRPTR
            parseptr(parser, lexer);
        }
    }
}

static void parsejmp(struct Parser *parser, struct Lexer *lexer, uint8_t opcode) {
    uint32_t start = emitter.written;
    emitbyte(&emitter, opcode);
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

static void parseset(struct Parser *parser, struct Lexer *lexer, uint8_t offset) {
    uint32_t start = emitter.written;
    emitbyte(&emitter, 0x1D); // SET
    // DEST
    if(checktoken(parser, TOK_REGISTER)) {
        emitbyte(&emitter, 0x00 + offset); // REG
        emitbyte(&emitter, resolveregister(parser->curtoken.text));
        match(parser, lexer, TOK_REGISTER);
    } else if(checktoken(parser, TOK_LSQUARE)) {
        emitbyte(&emitter, 0x01 + offset); // PTR
        parseptr(parser, lexer);
    }

}

static void parseint(struct Parser *parser, struct Lexer *lexer) {
    emitbyte(&emitter, 0x03); // INT
    if(checktoken(parser, TOK_NUMBER)) {
        emitbyte(&emitter, 0x02);
        emitbyte32(&emitter, strtol(parser->curtoken.text, NULL, 10));
        match(parser, lexer, TOK_NUMBER);
    } else if(checktoken(parser, TOK_CHAR)) {
        emitbyte(&emitter, 0x02);
        emitbyte32(&emitter, (char)parser->curtoken.text[0]);
        match(parser, lexer, TOK_CHAR);
    } else if(checktoken(parser, TOK_LABEL)) {
        emitbyte(&emitter, 0x02);
        parsesymbol(parser, lexer);
    } else if(checktoken(parser, TOK_REGISTER)) {
        emitbyte(&emitter, 0x00);
        emitbyte(&emitter, resolveregister(parser->curtoken.text));
        match(parser, lexer, TOK_REGISTER);
    } else if(checktoken(parser, TOK_LSQUARE)) {
        emitbyte(&emitter, 0x01);
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

static void parsegenerictwoargnostore(struct Parser *parser, struct Lexer *lexer, uint8_t opcode) {
    emitbyte(&emitter, opcode);
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

static void parsegenerictwoarg(struct Parser *parser, struct Lexer *lexer, uint8_t opcode) {
    emitbyte(&emitter, opcode);
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
        parsejmp(parser, lexer, 0x04);
    } else if(checktoken(parser, TOK_JNZ)) {
        nexttoken(parser, lexer);
        parsejmp(parser, lexer, 0x05);
    } else if(checktoken(parser, TOK_JZ)) {
        nexttoken(parser, lexer);
        parsejmp(parser, lexer, 0x06);
    } else if(checktoken(parser, TOK_JNE)) {
        nexttoken(parser, lexer);
        parsejmp(parser, lexer, 0x05);
    } else if(checktoken(parser, TOK_JE)) {
        nexttoken(parser, lexer);
        parsejmp(parser, lexer, 0x06);
    } else if(checktoken(parser, TOK_JL)) {
        nexttoken(parser, lexer);
        parsejmp(parser, lexer, 0x19);
    } else if(checktoken(parser, TOK_JLE)) {
        nexttoken(parser, lexer);
        parsejmp(parser, lexer, 0x1A);
    } else if(checktoken(parser, TOK_JG)) {
        nexttoken(parser, lexer);
        parsejmp(parser, lexer, 0x1B);
    } else if(checktoken(parser, TOK_JGE)) {
        nexttoken(parser, lexer);
        parsejmp(parser, lexer, 0x1C);
    } else if(checktoken(parser, TOK_SETEQ)) {
        nexttoken(parser, lexer);
        parseset(parser, lexer, 0x00);
    } else if(checktoken(parser, TOK_SETNE)) {
        nexttoken(parser, lexer);
        parseset(parser, lexer, 0x02);
    } else if(checktoken(parser, TOK_SETLT)) {
        nexttoken(parser, lexer);
        parseset(parser, lexer, 0x04);
    } else if(checktoken(parser, TOK_SETGT)) {
        nexttoken(parser, lexer);
        parseset(parser, lexer, 0x06);
    } else if(checktoken(parser, TOK_SETLE)) {
        nexttoken(parser, lexer);
        parseset(parser, lexer, 0x08);
    } else if(checktoken(parser, TOK_SETGE)) {
        nexttoken(parser, lexer);
        parseset(parser, lexer, 0x0A);
    } else if(checktoken(parser, TOK_LEA)) {
        nexttoken(parser, lexer);
        parselea(parser, lexer);
    } else if(checktoken(parser, TOK_CMP)) {
        nexttoken(parser, lexer);
        parsegenerictwoargnostore(parser, lexer, 0x12);
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
        parsegenerictwoargnostore(parser, lexer, 0x0A);
    } else if(checktoken(parser, TOK_POP)) {
        nexttoken(parser, lexer);
        parsepop(parser, lexer);
    } else if(checktoken(parser, TOK_PUSH)) {
        nexttoken(parser, lexer);
        parsepush(parser, lexer);
    } else if(checktoken(parser, TOK_ADD)) {
        nexttoken(parser, lexer);
        parsegenerictwoarg(parser, lexer, 0x0D); 
    } else if(checktoken(parser, TOK_SUB)) {
        nexttoken(parser, lexer);
        parsegenerictwoarg(parser, lexer, 0x0E);
    } else if(checktoken(parser, TOK_DIV)) {
        nexttoken(parser, lexer);
        parsegenerictwoarg(parser, lexer, 0x0F);
    } else if(checktoken(parser, TOK_IDIV)) {
        nexttoken(parser, lexer);
        parsegenerictwoarg(parser, lexer, 0x10);
    } else if(checktoken(parser, TOK_MUL)) {
        nexttoken(parser, lexer);
        parsegenerictwoarg(parser, lexer, 0x11);
    } else if(checktoken(parser, TOK_AND)) {
        nexttoken(parser, lexer);
        parsegenerictwoarg(parser, lexer, 0x13);
    } else if(checktoken(parser, TOK_SHL)) {
        nexttoken(parser, lexer);
        parsegenerictwoarg(parser, lexer, 0x14);
    } else if(checktoken(parser, TOK_SHR)) {
        nexttoken(parser, lexer);
        parsegenerictwoarg(parser, lexer, 0x15);
    } else if(checktoken(parser, TOK_XOR)) {
        nexttoken(parser, lexer);
        parsegenerictwoarg(parser, lexer, 0x16);
    } else if(checktoken(parser, TOK_OR)) {
        nexttoken(parser, lexer);
        parsegenerictwoarg(parser, lexer, 0x17);
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

#define MFMAGIC 0x160F106F
static void make_mf(uint32_t offset) {
    emitbyte32(&emitter, MFMAGIC);
    emitbyte32(&emitter, 0);
    labelmapent_t *label = labelmapget(labels, "main"); // entry
    if(!(label == NULL)) {
        emitbyte32(&emitter, label->value);
    } else {
        uint32_t current = emitter.written;
        emitbyte32(&emitter, 0x00000000); // emit zeros (replace later)
        struct Relocatable *relocatable = (struct Relocatable *)malloc(sizeof(struct Relocatable *)); // 32 bit relocatable located at this address.
        relocatable->symbol = "main";
        relocatable->loc = current;
        relocatable->mode = 0x03; // 32-bit
        relocatables[relocatablepointer++] = relocatable;
    }
    emitbyte32(&emitter, offset); // offset
}

int compiler(char *buffer, char *output, uint32_t offset, uint32_t basesize, uint8_t format) { 
    initemitter(&emitter, output);
    emitter.offset = offset;
    emitter.basesize = basesize;
    labels = labelmapcreate();

    if(format == 1) { // MF (Incredibly basic format for keeping track of offset and entry functions)
        make_mf(offset);
    }
    
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
                    relocatebyte32(&emitter, loc, labelloc);
                    break;
                case 0x04: // negative 8 bit
                    relocatebyte(&emitter, loc, -labelloc);
                    break;
                case 0x05: // negative 16 bit
                    relocatebyte16(&emitter, loc, -labelloc);
                    break;
                case 0x06: // negative 32 bit
                    relocatebyte32(&emitter, loc, -labelloc);
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
