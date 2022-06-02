#ifndef __DATAMAP_H__
#define __DATAMAP_H__

#include <stdint.h>
#include <stddef.h>

#define INITIAL_CAPACITY 1024

typedef struct labelmap labelmap_t;
typedef struct labelmapent {
    char *key;
    uint32_t value;
} labelmapent_t;

/*struct LabelMapPair {
    char *key;
    uint32_t value;
};

void labelmapput(struct LabelMapPair *map[], char *key, uint32_t value);
struct LabelMapPair *labelmapget(struct LabelMapPair *map[], char *key);*/

typedef struct {
    char *key;
    uint32_t value;

    labelmap_t *_map;
    size_t _index;
} labelmapi_t;

labelmap_t *labelmapcreate(void);
void labelmapdestroy(labelmap_t *map);
labelmapent_t *labelmapget(labelmap_t *map, char *key);
char *labelmapset(labelmap_t *map, char *key, uint32_t value);
size_t labelmaplen(labelmap_t *map);

labelmapi_t labelmapiterator(labelmap_t *map);
int labelmapnext(labelmapi_t *it);

#endif
