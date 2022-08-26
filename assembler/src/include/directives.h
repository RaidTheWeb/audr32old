#ifndef __DIRECTIVES_H__
#define __DIRECTIVES_H__

#include "compiler.h"
#include "parser.h"

void parsemacro(struct Parser *parser, struct Lexer *lexer);
void parseperiod(struct Parser *parser, struct Lexer *lexer);

#endif
