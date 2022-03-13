#ifndef __EMITTER_H__
#define __EMITTER_H__

struct Emitter {
    char fullPath[1024];
    uint8_t *code;
    uint32_t cp;
    uint32_t written;
};

void initemitter(struct Emitter *emitter, char *fullPath);

void emitbyte(struct Emitter *emitter, uint8_t byte);
void emitbyte16(struct Emitter *emitter, uint16_t byte16);
void emitbyte32(struct Emitter *emitter, uint32_t byte32);

void relocatebyte(struct Emitter *emitter, uint32_t loc, uint8_t byte);
void relocatebyte16(struct Emitter *emitter, uint32_t loc, uint16_t byte16);
void relocatebyte32(struct Emitter *emitter, uint32_t loc, uint32_t byte32);

void writefile(struct Emitter *emitter);

#endif