#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <time.h>

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