#ifndef __COMMON_H__
#define __COMMON_H__

#include <stdint.h>

#define BYTE 1
#define KB BYTE * 1024
#define MB KB * 1024
#define GB MB * 1024
#define TB GB * 1024


uint32_t swapendian(uint32_t num);
uint16_t ensurebig16(uint16_t num);
uint32_t ensurebig32(uint32_t num);
uint32_t ensurelittle32(uint32_t num);
uint32_t hashstring(char *str);
char *getfilebase(char *path);
char *removeext(char *filename);
char *concatstr(const char *prefix, const char *suffix);
int msleep(long tms);

#endif
