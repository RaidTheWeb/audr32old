#ifndef __DATAMAP_H__
#define __DATAMAP_H__

#include <stdint.h>

#define SIZE 256

struct LabelMapPair {
    char *key;
    uint32_t value;
};

void labelmapput(struct LabelMapPair *map[], char *key, uint32_t value);
struct LabelMapPair *labelmapget(struct LabelMapPair *map[], char *key);

#endif