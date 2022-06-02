#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "common.h"
#include "datamap.h"

/*static int hashcode(char *key) {
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
    if(map[index] != NULL) {
        printf("label resolution for %s at %d\n", key, index);
        return map[index];
    }
    
    return NULL;
}

struct LabelMapPair *labelmapdelete(struct LabelMapPair *map[], struct LabelMapPair *item) {
    char *key = item->key;

    int index = hashcode(key);
    map[index] = nullitem;

    return NULL;
}*/



struct labelmap {
    labelmapent_t *entries;
    size_t capacity;
    size_t length;
};

labelmap_t *labelmapcreate(void) {
    labelmap_t *table = malloc(sizeof(labelmap_t));
    if(table == NULL) {
        printf("failed to allocate memory for labelmap!\n");
        exit(1);
    }
    table->length = 0;
    table->capacity = INITIAL_CAPACITY;

    table->entries = calloc(table->capacity, sizeof(labelmapent_t));
    if(table->entries == NULL) {
        free(table);
        printf("failed to allocate memory for labelmap entries!\n");
        exit(1);
    }

    return table;
}

void labelmapdestroy(labelmap_t *map) {
    for(size_t i = 0; i < map->capacity; i++) {
        free((void*)map->entries[i].key);
    }
    
    free(map->entries);
    free(map);
}

labelmapent_t *labelmapget(labelmap_t *map, char *key) {
    uint32_t hash = hashstring(key);
    size_t index = (size_t)(hash & (uint32_t)(map->capacity - 1));

    while(map->entries[index].key != NULL) {
        if(strcmp(key, map->entries[index].key) == 0) {
            return &map->entries[index];
        }

        index++;
        if(index >= map->capacity) {
            index = 0;
        }
    }

    return NULL;
}

static char *labelmapsetent(labelmapent_t *entries, size_t capacity, char *key, uint32_t value, size_t *plength) {
    uint32_t hash = hashstring(key);
    size_t index = (size_t)(hash & (uint32_t)(capacity - 1));

    while(entries[index].key != NULL) {
        if(strcmp(key, entries[index].key) == 0) {
            entries[index].value = value;
            return entries[index].key;
        }

        index++;

        if(index >= capacity) {
            index = 0;
        }
    }

    if(plength != NULL) {
        key = strdup(key);

        if(key == NULL) return NULL;
        (*plength)++;
    }
    entries[index].key = key;
    entries[index].value = value;
    return key;
}


char *labelmapset(labelmap_t *map, char *key, uint32_t value) {
    if(value == -1) {
        return NULL;
    }

    if(map->length >= map->capacity) {
        return NULL;
    }

    return labelmapsetent(map->entries, map->capacity, key, value, &map->length);
}

size_t labelmaplen(labelmap_t *map) {
    return map->length;
}

labelmapi_t labelmapiterator(labelmap_t *map) {
    labelmapi_t it;
    it._map = map;
    it._index = 0;
    return it;
}

int labelmapnext(labelmapi_t *it) {
    labelmap_t *map = it->_map;
    while(it->_index < map->capacity) {
        size_t i = it->_index;
        it->_index++;
        if(map->entries[i].key != NULL) {
            labelmapent_t entry = map->entries[i];
            it->key = entry.key;
            it->value = entry.value;
            return 1;
        }
    }
    return 0;
}
