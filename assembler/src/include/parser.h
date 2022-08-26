#ifndef __PARSER_H__
#define __PARSER_H__

#include <stdint.h>

#include "compiler.h"

extern int mode;

uint8_t resolveregister(char *text);
char *resolvetoken(int type);
int checktoken(struct Parser *parser, int type);
int checkpeek(struct Parser *parser, int type);
void parserabort(char *message);
void nexttoken(struct Parser *parser, struct Lexer *lexer);
void match(struct Parser *parser, struct Lexer *lexer, int type);
void checknewline(struct Parser *parser, struct Lexer *lexer);
void parsesymbol(struct Parser *parser, struct Lexer *lexer);
void parsesymbolneg(struct Parser *parser, struct Lexer *lexer);
void parseptr(struct Parser *parser, struct Lexer *lexer);

#endif
