#include <errno.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>

#include "compiler.h"
#include "emitter.h"
#include "datamap.h"
#include "lexer.h"
#include "parser.h"

// section
int mode = 0; // text (0 text, 1 data)


// ---------
//  MACRO
// ---------

void compilefile(char *buffer);

void parsemacro(struct Parser *parser, struct Lexer *lexer) {
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

    } else if(strcmp(macroname.text, "org") == 0) { // data origin (offset)
        if(checktoken(parser, TOK_NUMBER)) {
            // old logic (physical location inside exectuable)
            // emitter.cp = strtol(parser->curtoken.text, NULL, 10);
            // if(emitter.cp > emitter.written) emitter.written = emitter.cp;
            emitter.offset = strtol(parser->curtoken.text, NULL, 10); // manually set an offset
            match(parser, lexer, TOK_NUMBER);
        }
    } else if(strcmp(macroname.text, "include") == 0) { // include assembly file 
        struct Token constant = parser->curtoken;
        match(parser, lexer, TOK_STRING);
        FILE *file = fopen(constant.text, "r");
        if(file == NULL) {
            printf("Failed to include `%s` because an error occurred. (%s)", constant.text, strerror(errno));
            exit(1);
        }

        fseek(file, 0, SEEK_END);
        size_t size = ftell(file);
        fseek(file, 0, SEEK_SET);

        char *buffer = (char *)malloc(size);

        size_t read = fread(buffer, sizeof(uint8_t), size, file);

        fclose(file);
        buffer[size] = '\0';
        printf("include size: %s %zu\n", constant.text, strlen(buffer));

        compilefile(buffer);

    } else if(strcmp(macroname.text, "incbin") == 0) { // include a binary
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

void parseperiod(struct Parser *parser, struct Lexer *lexer) {
    struct Token operator = parser->curtoken;
    match(parser, lexer, TOK_LABEL);
    if(strcmp(operator.text, "text") == 0) { // code
        mode = 0;
        return;
    } else if(strcmp(operator.text, "data") == 0) { // raw data
        mode = 1;
        return;
    } else if(strcmp(operator.text, "global") == 0) { // global (ignore for now)
        match(parser, lexer, TOK_LABEL); // symbol to make global :rolling_eyes:
        return;
    }

    if(mode == -1) {
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
