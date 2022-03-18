#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <time.h>

uint32_t swapendian(uint32_t num) {
    uint32_t b0,b1,b2,b3;
    uint32_t res;

    b0 = (num & 0x000000FF) << 24u;
    b1 = (num & 0x0000FF00) << 8u;
    b2 = (num & 0x00FF0000) << 8u;
    b3 = (num & 0xFF000000) >> 24u;

    res = b0 | b1 | b2 | b3;

    return res;
}

uint16_t ensurebig16(uint16_t num) {
    uint32_t i = 1;
    char *c = (char *)&i;
    if(*c)
        return swapendian(num);
    return num;
}

uint32_t ensurebig32(uint32_t num) {
    uint32_t i = 1;
    char *c = (char *)&i;
    if(*c)
        return swapendian(num);
    return num;
}

uint32_t hashstring(char *str) {
    uint32_t hash = 2166136261u;
    for(int i = 0; i < strlen(str); i++) {
        hash ^= (uint8_t)str[i];
        hash *= 16777619;
    }
    return (int)hash;
}

char* getfilebase(char *path) {
    for(size_t i = strlen(path) - 1; i; i--) {
        if (path[i] == '/') {
            return &path[i+1];
        }
    }
    return path;
}

char *removeext(char *filename) {
    char *retStr;
    char *lastExt;
    if (filename == NULL) return NULL;
    if ((retStr = malloc(strlen(filename) + 1)) == NULL) return NULL;
    strcpy(retStr, filename);
    lastExt = strrchr(retStr, '.');
    if (lastExt != NULL)
        *lastExt = '\0';
    return retStr;
}


char *concatstr(const char *prefix, const char *suffix) {
    char *result = malloc(strlen(prefix) + strlen(suffix) + 1);
    strcpy(result, prefix);
    strcat(result, suffix);

    return result;
}

int msleep(long tms)
{
    struct timespec ts;
    int ret;

    if (tms < 0)
    {
        errno = EINVAL;
        return -1;
    }

    ts.tv_sec = tms / 1000;
    ts.tv_nsec = (tms % 1000) * 1000000;

    do {
        ret = nanosleep(&ts, &ts);
    } while (ret && errno == EINTR);

    return ret;
}
