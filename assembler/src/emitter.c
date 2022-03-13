#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common.h"
#include "emitter.h"

/*********************************************************************
 * Audr32 Assembler
 * emitter.c - Emit data to a buffer, contains relocation and emit functions.
 *
 * Do everything needed in here. >//<
 * Assembler specification is in `../ASM.txt`
 *********************************************************************/

void initemitter(struct Emitter *emitter, char *fullPath) {
    strcpy(emitter->fullPath, fullPath);
    emitter->code = (uint8_t *)malloc(0xFFFF * sizeof(uint8_t));
    emitter->cp = 0;
    emitter->written = 0;
}

void emitbyte(struct Emitter *emitter, uint8_t byte) {
    emitter->code[emitter->cp++] = byte;
    emitter->written++;
}

void emitbyte16(struct Emitter *emitter, uint16_t byte16) {
    emitbyte(emitter, byte16 >> 8);
    emitbyte(emitter, byte16);
}

void emitbyte32(struct Emitter *emitter, uint32_t byte32) {
    emitbyte16(emitter, byte32 >> 16);
    emitbyte16(emitter, byte32);
}

void relocatebyte(struct Emitter *emitter, uint32_t loc, uint8_t byte) {
    emitter->code[loc] = byte;
}

void relocatebyte16(struct Emitter *emitter, uint32_t loc, uint16_t byte16) {
    relocatebyte(emitter, loc, byte16 >> 8);
    relocatebyte(emitter, loc + 1, byte16);
}

void relocatebyte32(struct Emitter *emitter, uint32_t loc, uint32_t byte32) {
    relocatebyte(emitter, loc, byte32 >> 24);
    relocatebyte(emitter, loc + 1, byte32 >> 16);
    relocatebyte(emitter, loc + 2, byte32 >> 8);
    relocatebyte(emitter, loc + 3, byte32);
}

void writefile(struct Emitter *emitter) {
    FILE *fp = fopen(emitter->fullPath, "w+b");
    fwrite(emitter->code, sizeof(uint8_t), 0xFFFF, fp);
    fclose(fp);
}