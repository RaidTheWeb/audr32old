#ifndef __COMMON_H__
#define __COMMON_H__

#include <stdint.h>

uint32_t hashstring(char *str);
char *getfilebase(char *path);
char *removeext(char *filename);
char *concatstr(const char *prefix, const char *suffix);
char *substring(char *string, int position, int length);
char *itoa (uint64_t value, char str[], int radix);

#endif