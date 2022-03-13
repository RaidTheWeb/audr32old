#ifndef __COMPILER_H__
#define __COMPILER_H__

#include <stdint.h>
#include <stdlib.h>

#define ERRNOFILE 0x10          // file does not exist
#define ERRNOCONTENT 0x11       // file is empty (no contents)

typedef struct {
    //uint32_t count;
    //uint32_t capacity;
    char *outputfile;
} compilerjob_t;

#define GROW_BUFFER(compilerjob) compilerjob.buffer = realloc(compilerjob.buffer, ++compilerjob.capacity)
#define GROW_CAPACITY(capacity) (++capacity)

int compiler(char *source, char *output);

#endif