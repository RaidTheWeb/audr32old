#include <ctype.h>
#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <stdlib.h>

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

char *substring(char *string, int position, int length) {
    char *p;
    int c;
 
    p = malloc(length + 1);
   
    if (p == NULL) {
        printf("Unable to allocate memory.\n");
        exit(1);
    }
 
    for (c = 0; c < length; c++) {
        *(p+c) = *(string + position - 1);
        string++;  
    }
 
    *(p+c) = '\0';
 
    return p;
}

char *itoa (uint64_t value, char str[], int radix) {
    char buf[66];
    char *dest = buf + sizeof(buf);
    int sign = 0;

    if(value == 0) {
        memcpy(str, "0", 2);
        return str;
    }

    if(radix < 0) {
        radix = -radix;
        if((long long)value < 0) {
            value = -value;
            sign = 1;
        }
    }

    *--dest = '\0';

    switch (radix) {
        case 16:
            while(value) {
                * --dest = '0' + (value & 0xF);
                if(*dest > '9') *dest += 'A' - '9' - 1;
                value >>= 4;
            }
            break;
        case 10:
            while(value) {
                *--dest = '0' + (value % 10);
                value /= 10;
            }
            break;

        case 8:
            while(value) {
                *--dest = '0' + (value & 7);
                value >>= 3;
            }
            break;

        case 2:
            while(value) {
                *--dest = '0' + (value & 1);
                value >>= 1;
            }
            break;

        default:            // The slow version, but universal
            while (value) {
                *--dest = '0' + (value % radix);
                if (*dest > '9') *dest += 'A' - '9' - 1;
                value /= radix;
            }
            break;
    }

    if(sign) *--dest = '-';

    memcpy(str, dest, buf +sizeof(buf) - dest);
    return str;
}
