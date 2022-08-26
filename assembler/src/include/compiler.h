#ifndef __COMPILER_H__
#define __COMPILER_H__

#include <stdint.h>
#include <stdlib.h>

#include "datamap.h"
#include "lexer.h"

#define ERRNOFILE 0x10          // file does not exist
#define ERRNOCONTENT 0x11       // file is empty (no contents)

typedef struct {
    char *outputfile;
} compilerjob_t;

struct Relocatable {
    char *symbol;
    uint32_t loc; // location where to relocate
    uint8_t mode; // 0x01, 0x02, 0x03, 0x04, 0x05, 0x06 (8 bit, 16 bit, 32 bit, negative 8 bit, negative 16 bit, and negative 32 bit respectively)
};

struct Parser {
    struct Token curtoken;
    struct Token peektoken;
};

extern struct Emitter emitter;
extern labelmap_t *labels;
extern struct Relocatable *relocatables[INITIAL_CAPACITY];
extern int relocatablepointer;

int checktoken(struct Parser *parser, int type);
int checkpeek(struct Parser *parser, int type);
void parserabort(char *message);
void nexttoken(struct Parser *parser, struct Lexer *lexer);
void match(struct Parser *parser, struct Lexer *lexer, int type);
void checknewline(struct Parser *parser, struct Lexer *lexer);
int compiler(char *source, char *output, uint32_t offset, uint32_t basesize, uint8_t format);

#endif
