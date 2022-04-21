#include <stdint.h>
#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <time.h>

#include "common.h"

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

uint32_t ensurelittle32(uint32_t num) {
    uint32_t i = 1;
    char *c = (char *)&i;
    if(*c) { // little endian, ensure swapped to little endian because no data passed in here should be little endian in the first place.
        return swapendian(num); // swap to little endian as the way memory is stored forces it.
    } else {
        // do nothing because it's big endian (meaning that ensurance later will not cause issues.)
        return num; // we do not need to swap our big endian input data to little endian because data storage is already big endian, the later ensurebig32() call will not need to do anything for us, causing no issue further down the line.
    }

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

int msleep(long tms) {
    struct timespec ts;
    int ret;

    if (tms < 0) {
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

uint32_t getpixel(image_t image, uint32_t x, uint32_t y) {
    if(x > image.w) x = image.w - 1;
    if(y > image.h) y = image.h - 1;
    uint32_t index = x * (32/8) +
        y * (image.w*(32/8));
    //printf("0x%08x\n", image.pixeldata[index]);
    return image.pixeldata[index];
}

static size_t cursor;

static int readnext(uint8_t *imagedata, uint32_t bytes) {
	int total = 0;

	for(uint32_t i = 0; i < bytes; i++) {
		total += (imagedata[cursor++] << (8 * i));
	}

	return total;
}

void initimage(image_t *image, uint8_t *imagedata, size_t length) {
    uint8_t currentByte;
    
    size_t bfOffBits;
    size_t biSizeImage;

    while(cursor <= length) {
        currentByte = imagedata[cursor++];

        if(cursor == 11)
            bfOffBits = currentByte + readnext(imagedata, 3);
        else if(cursor == 19)
            image->w = currentByte + readnext(imagedata, 3);
        else if(cursor == 23)
            image->h = currentByte + readnext(imagedata, 3);
        else if(cursor == 35)
            biSizeImage = currentByte + readnext(imagedata, 3);

        else if(cursor == (bfOffBits + 1)) {
            printf("%lu\n", biSizeImage);
            image->pixeldata = (uint32_t *)malloc(image->w * image->h);
            image->pixeldata[0] = currentByte + readnext(imagedata, 3);

            for(uint32_t i = 1; i < biSizeImage; i++) {
                image->pixeldata[i] = imagedata[cursor] + readnext(imagedata, 3);
                printf("0x%08x ", image->pixeldata[i]);
            }
        }
    } 
}
