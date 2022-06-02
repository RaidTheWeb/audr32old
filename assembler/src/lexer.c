#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common.h"
#include "lexer.h"

/*********************************************************************
 * Audr32 Assembler
 * lexer.c - Synatatic Analysis functions.
 *
 * Do everything needed in here. >//<
 * Assembler specification is in `../ASM.txt`
 *********************************************************************/

void inittoken(struct Token *token, char *text, int type) {
    strcpy(token->text, text);
    token->type = type;
}

void inittokenc(struct Token *token, char text, int type) {
    char tmp[2];
    tmp[0] = text;
    tmp[1] = '\0';
    strcpy(token->text, tmp);
    token->type = type;
}

#define DEF_REG(_n) \
    else if(strcmp(text, #_n) == 0) { \
        return TOK_REGISTER; \
    }

int checkifkeyword(char *text, struct Lexer *lexer, int startofline) {
    if(startofline && peek(lexer) == ':') return TOK_LABEL; // allow definition of labels named the same as instructions
    DEF_REG(ax) // registers have next level precedence
    DEF_REG(bx)
    DEF_REG(cx)
    DEF_REG(dx)
    DEF_REG(si)
    DEF_REG(di)
    DEF_REG(sp)
    DEF_REG(bp)
    DEF_REG(ip)
    DEF_REG(r0)
    DEF_REG(r1)
    DEF_REG(r2)
    DEF_REG(r3)
    DEF_REG(r4)
    DEF_REG(r5)
    DEF_REG(r6)
    DEF_REG(r7)
    DEF_REG(r8)
    DEF_REG(r9)
    DEF_REG(r10)
    DEF_REG(r11)
    DEF_REG(r12)
    DEF_REG(r13)
    DEF_REG(r14)
    DEF_REG(r15)
    else if(!startofline) return TOK_LABEL; // allow usage

    if(strcmp(text, "noop") == 0) {
        return TOK_NOOP;
    } else if(strcmp(text, "halt") == 0) {
        return TOK_HALT;
    } else if(strcmp(text, "mov") == 0) {
        return TOK_MOV;
    } else if(strcmp(text, "int") == 0) {
        return TOK_INT;
    } else if(strcmp(text, "jmp") == 0) {
        return TOK_JMP;
    } else if(strcmp(text, "jnz") == 0) {
        return TOK_JNZ;
    } else if(strcmp(text, "jz") == 0) {
        return TOK_JZ;
    } else if(strcmp(text, "jne") == 0) {
        return TOK_JNE;
    } else if(strcmp(text, "je") == 0) {
        return TOK_JE;
    } else if(strcmp(text, "jl") == 0) {
        return TOK_JL;
    } else if(strcmp(text, "jle") == 0) {
        return TOK_JLE;
    } else if(strcmp(text, "jg") == 0) {
        return TOK_JG;
    } else if(strcmp(text, "jge") == 0) {
        return TOK_JGE;
    } else if(strcmp(text, "seteq") == 0) {
        return TOK_SETEQ;
    } else if(strcmp(text, "setne") == 0) {
        return TOK_SETNE;
    } else if(strcmp(text, "setlt") == 0) {
        return TOK_SETLT;
    } else if(strcmp(text, "setgt") == 0) {
        return TOK_SETGT;
    } else if(strcmp(text, "setle") == 0) {
        return TOK_SETLE;
    } else if(strcmp(text, "setge") == 0) {
        return TOK_SETGE;
    } else if(strcmp(text, "lea") == 0) {
        return TOK_LEA;
    } else if(strcmp(text, "call") == 0) {
        return TOK_CALL;
    } else if(strcmp(text, "ret") == 0) {
        return TOK_RET;
    } else if(strcmp(text, "inx") == 0) {
        return TOK_INX;
    } else if(strcmp(text, "outx") == 0) {
        return TOK_OUTX;
    } else if(strcmp(text, "pop") == 0) {
        return TOK_POP;
    } else if(strcmp(text, "push") == 0) {
        return TOK_PUSH;
    } else if(strcmp(text, "add") == 0) {
        return TOK_ADD;
    } else if(strcmp(text, "iadd") == 0) {
        return TOK_IADD;
    } else if(strcmp(text, "sub") == 0) {
        return TOK_SUB;
    } else if(strcmp(text, "isub") == 0) {
        return TOK_ISUB;
    } else if(strcmp(text, "div") == 0) {
        return TOK_DIV;
    } else if(strcmp(text, "idiv") == 0) {
        return TOK_IDIV;
    } else if(strcmp(text, "mul") == 0) {
        return TOK_MUL;
    } else if(strcmp(text, "imul") == 0) {
        return TOK_IMUL;
    } else if(strcmp(text, "inc") == 0) {
        return TOK_INC;
    } else if(strcmp(text, "dec") == 0) {
        return TOK_DEC;
    } else if(strcmp(text, "cmp") == 0) {
        return TOK_CMP;
    } else if(strcmp(text, "and") == 0) {
        return TOK_AND;
    } else if(strcmp(text, "shl") == 0) {
        return TOK_SHL;
    } else if(strcmp(text, "shr") == 0) {
        return TOK_SHR;
    } else if(strcmp(text, "xor") == 0) {
        return TOK_XOR;
    } else if(strcmp(text, "or") == 0) {
        return TOK_OR;
    } else if(strcmp(text, "not") == 0) {
        return TOK_NOT;
    } else if(strcmp(text, "neg") == 0) {
        return TOK_NEG;
    } else if(strcmp(text, "test") == 0) {
        return TOK_TEST;
    } else if(strcmp(text, "cld") == 0) {
        return TOK_CLD;
    } else if(strcmp(text, "lodsb") == 0) {
        return TOK_LODSB;
    } else if(strcmp(text, "lodsw") == 0) {
        return TOK_LODSW;
    } else if(strcmp(text, "lodsd") == 0) {
        return TOK_LODSD;
    } else if(strcmp(text, "loop") == 0) {
        return TOK_LOOP;
    } else if(strcmp(text, "pusha") == 0) {
        return TOK_PUSHA;
    } else if(strcmp(text, "popa") == 0) {
        return TOK_POPA;
    }
    
    return TOK_LABEL;
}

void nextchar(struct Lexer *, int);

void initlexer(struct Lexer *lexer, char *source) {
    lexer->source = (char *)malloc(strlen(source) + 1);
    strcpy(lexer->source, concatstr(source, "\n"));
    lexer->current = 0;
    lexer->pos = -1;
    
    lexer->line = 1;
    lexer->charpos = 0;

    nextchar(lexer, 0);
}

void nextchar(struct Lexer *lexer, int whitespace) {
    lexer->pos++;
    if(!whitespace) lexer->charpos++;// increment line character
    if(lexer->pos >= strlen(lexer->source)) lexer->current = '\0';
    else lexer->current = lexer->source[lexer->pos]; 
    if(lexer->current == '\n') {
        lexer->line++;
        lexer->charpos = 0;
    }
}

char peek(struct Lexer *lexer) {
    if(lexer->pos + 1 >= strlen(lexer->source)) return '\0';
    return lexer->source[lexer->pos + 1];
}

void skipwhitespace(struct Lexer *lexer) {
    while(lexer->current == ' ' || lexer->current == '\t' || lexer->current == '\r' || lexer->current == '\f' || ((lexer->current > 0 && lexer->current < 9) || (lexer->current > 14 && lexer->current < 32))) { // || lexer->current < 0x20) { 
//        if(lexer->current == '\n' && lexer->current == '\0') break; // catch newlines and EOF
        nextchar(lexer, 1);
    }
}

void skipcomment(struct Lexer *lexer) {
    if(lexer->current == ';') {
        while(lexer->current != '\n') nextchar(lexer, 1);
        
    }
}

void lexerabort(struct Lexer *lexer, char *message) {
    printf("Assembler lexer error: %s (Line %lu, Character %lu (0x%02x))\n", message, lexer->line, lexer->charpos, lexer->current);
    exit(1);
}

struct Token gettoken(struct Lexer *lexer) { 
    skipwhitespace(lexer);
    skipcomment(lexer);
    struct Token token;

    if(lexer->current == '+')
        inittokenc(&token, lexer->current, TOK_PLUS);
    else if(lexer->current == '-') {
        if (isdigit(peek(lexer))) {
            nextchar(lexer, 0);
            if(peek(lexer) == 'x') {
                nextchar(lexer, 0);
                nextchar(lexer, 0);
                int startpos = lexer->pos;
                while(isxdigit(peek(lexer)))
                    nextchar(lexer, 0);

                int32_t num = -strtol(substring(lexer->source, startpos + 1, lexer->pos - (startpos - 1)), NULL, 16);
                char buffer[256];
                itoa(num, buffer, 10);


                inittoken(&token, buffer, TOK_NUMBER);
            } else if(peek(lexer) == 'b') {
                nextchar(lexer, 0);
                nextchar(lexer, 0);
                int startpos = lexer->pos;
                while(isdigit(peek(lexer)))
                    nextchar(lexer, 0);

                int32_t num = -strtol(substring(lexer->source, startpos + 1, lexer->pos - (startpos - 1)), NULL, 2);
                char buffer[256];
                itoa(num, buffer, 10);


                inittoken(&token, buffer, TOK_NUMBER);
            } else if(peek(lexer) == 'o') {
                nextchar(lexer, 0);
                nextchar(lexer, 0);
                int startpos = lexer->pos;
                while(isdigit(peek(lexer)))
                    nextchar(lexer, 0);

                int32_t num = -strtol(substring(lexer->source, startpos + 1, lexer->pos - (startpos - 1)), NULL, 8);
                char buffer[256];
                itoa(num, buffer, 10);


                inittoken(&token, buffer, TOK_NUMBER);
            } else {
                int startpos = lexer->pos - 1;

                while(isdigit(peek(lexer)))
                    nextchar(lexer, 0);

                inittoken(&token, substring(lexer->source, startpos + 1, lexer->pos - (startpos - 1)), TOK_NUMBER);
            }
        } else {
            inittokenc(&token, lexer->current, TOK_MINUS);
        }
    } 
    else if(lexer->current == '*')
        inittokenc(&token, lexer->current, TOK_ASTERISK);
    else if(lexer->current == '/')
        inittokenc(&token, lexer->current, TOK_SLASH);
    else if(lexer->current == ',')
        inittokenc(&token, lexer->current, TOK_COMMA);
    else if(lexer->current == ':')
        inittokenc(&token, lexer->current, TOK_COLON);
    else if(lexer->current == '|')
        inittokenc(&token, lexer->current, TOK_BOR);
    else if(lexer->current == '&')
        inittokenc(&token, lexer->current, TOK_BAND);
    else if(lexer->current == '[')
        inittokenc(&token, lexer->current, TOK_LSQUARE);
    else if(lexer->current == ']')
        inittokenc(&token, lexer->current, TOK_RSQUARE);
    else if(lexer->current == '(')
        inittokenc(&token, lexer->current, TOK_LBRACKET);
    else if(lexer->current == ')')
        inittokenc(&token, lexer->current, TOK_RBRACKET);
    else if(lexer->current == '#')
        inittokenc(&token, lexer->current, TOK_HASHTAG);
    else if(lexer->current == '.')
        inittokenc(&token, lexer->current, TOK_PERIOD);
    else if(lexer->current == '$')
        inittokenc(&token, lexer->current, TOK_DOLLAR); // basically the only thing we'll use it for
    else if(lexer->current == '\"') {
        nextchar(lexer, 0);
        int startpos = lexer->pos;
        int marked = 0;

        while(!(!marked && lexer->current == '\"')) {
            marked = 0;
            if(lexer->current == '\r' || lexer->current == '\n' || lexer->current == '\t')
                lexerabort(lexer, "Illegal character in string");
            if(lexer->current == '\\')
                marked = 1;
            nextchar(lexer, 0);
        }
        inittoken(&token, substring(lexer->source, startpos + 1, lexer->pos - startpos), TOK_STRING);
    } else if(lexer->current == '\'') {
        nextchar(lexer, 0);
        int startpos = lexer->pos;
        int allowed = 1;
        int marked = 0;

        while(!(!marked && lexer->current == '\'')) {
            marked = 0;
            if(!isascii(lexer->current))
                lexerabort(lexer, "Character exceeds ASCII range (0-255)");
            if(lexer->current == '\r' || lexer->current == '\n' || lexer->current == '\t')
                lexerabort(lexer, "Illegal escape in character");
            if(lexer->current == '\\') {
                marked = 1;
                allowed = 2;
            }
            nextchar(lexer, 0);
        }

        if(lexer->pos - startpos > allowed)
            lexerabort(lexer, "Character of illegal length");
        inittoken(&token, substring(lexer->source, startpos + 1, lexer->pos - (startpos)), TOK_CHAR);
    } else if(isdigit(lexer->current)) {
        if(peek(lexer) == 'x') {

            nextchar(lexer, 0);
            nextchar(lexer, 0);
            int startpos = lexer->pos;
            while(isxdigit(peek(lexer)))
                nextchar(lexer, 0);

            uint32_t num = strtol(substring(lexer->source, startpos + 1, lexer->pos - (startpos - 1)), NULL, 16);
            char buffer[256];
            itoa(num, buffer, 10);


            inittoken(&token, buffer, TOK_NUMBER);
        } else if(peek(lexer) == 'b') {
            nextchar(lexer, 0);
            nextchar(lexer, 0);
            int startpos = lexer->pos;
            while(isdigit(peek(lexer)))
                nextchar(lexer, 0);

            uint32_t num = strtol(substring(lexer->source, startpos + 1, lexer->pos - (startpos - 1)), NULL, 2);
            char buffer[256];
            itoa(num, buffer, 10);


            inittoken(&token, buffer, TOK_NUMBER);
        } else if(peek(lexer) == 'o') {
            nextchar(lexer, 0);
            nextchar(lexer, 0);
            int startpos = lexer->pos;
            while(isdigit(peek(lexer)))
                nextchar(lexer, 0);

            uint32_t num = strtol(substring(lexer->source, startpos + 1, lexer->pos - (startpos - 1)), NULL, 8);
            char buffer[256];
            itoa(num, buffer, 10);


            inittoken(&token, buffer, TOK_NUMBER);
        } else {
            int startpos = lexer->pos;

            while(isdigit(peek(lexer)))
                nextchar(lexer, 0);
            
            inittoken(&token, substring(lexer->source, startpos + 1, lexer->pos - (startpos - 1)), TOK_NUMBER);
        }
    } else if(isalpha(lexer->current)) {
        int startpos = lexer->pos;
        int startofline = 0;
        if(lexer->charpos == 1) startofline = 1; // so we know if we start at the beginning or not
        while(isalnum(peek(lexer)) || peek(lexer) == '_')
            nextchar(lexer, 0);

        char *text = substring(lexer->source, startpos + 1, (lexer->pos) - (startpos - 1));

        int type = checkifkeyword(text, lexer, startofline);
        inittoken(&token, text, type);
    } else if(lexer->current == '\n') {
        inittokenc(&token, lexer->current, TOK_NEWLINE); 
    } else if(lexer->current == '\0') {
        inittokenc(&token, '\0', TOK_EOF);
    } else if((lexer->current > 0 && lexer->current < 9) || (lexer->current > 14 && lexer->current < 32)) {
        printf("excluded character\n");
    } else {
        lexerabort(lexer, "Unknown Token");
    }
    
    nextchar(lexer, 0);
    return token;
}
