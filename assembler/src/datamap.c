#include <stdint.h>
#include <stdlib.h>

#include "common.h"
#include "datamap.h"

static int hashcode(char *key) {
    return hashstring(key) % SIZE;
}

struct LabelMapPair *nullitem;

void labelmapput(struct LabelMapPair *map[], char *key, uint32_t value) {
    struct LabelMapPair *item = (struct LabelMapPair *)malloc(sizeof(struct LabelMapPair));
    item->value = value;
    item->key = key;

    int index = hashcode(key);

    map[index] = item;
}

struct LabelMapPair *labelmapget(struct LabelMapPair *map[], char *key) {
    int index = hashcode(key);
    if(map[index] != NULL)
        return map[index];
    
    return NULL;
}

struct LabelMapPair *labelmapdelete(struct LabelMapPair *map[], struct LabelMapPair *item) {
    char *key = item->key;

    int index = hashcode(key);
    map[index] = nullitem;

    return NULL;
}