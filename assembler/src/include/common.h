#ifndef __COMMON_H__
#define __COMMON_H__

#include <stdint.h>

uint16_t ensurebig16(uint16_t num);
uint32_t ensurebig32(uint32_t num);

uint32_t hashstring(char *str);
char *getfilebase(char *path);
char *removeext(char *filename);
char *concatstr(const char *prefix, const char *suffix);
char *substring(char *string, int position, int length);
char *itoa (uint64_t value, char str[], int radix);

#endif
