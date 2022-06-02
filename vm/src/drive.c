#include <errno.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "bus.h"
#include "drive.h"
#include "interrupts.h"

typedef struct {
    FILE *image;
    int id;
    int set;
    uint32_t blocks;
} drive_t;

drive_t drives[DRIVES];

int info = 0;
int details = 0;

drive_t *seldrive = 0;

uint32_t moda = 0;
uint32_t modb = 0;

int dointerrupt = 0;

void interrupt_trigger(uint16_t from, uint16_t to);

static void drive_info(int dinfo, int ddetails) {
    info = dinfo;
    details = ddetails;

    if(dointerrupt) interrupt_trigger(0, 0x0E);
}

uint8_t blockbuffer[512];

enum {
    DRIVE_SELDRIVE,
    DRIVE_READBLK,
    DRIVE_WRITEBLK,
    DRIVE_INFO,
    DRIVE_POLL,
    DRIVE_DOINT,
    DRIVE_DONTINT
};

static void drive_writecmd(uint16_t port, uint32_t value) {
    switch(value) {
        case DRIVE_SELDRIVE: {
            if((moda < DRIVES) && (drives[moda].set)) seldrive = &drives[moda];
            else seldrive = 0;
            break;
        }
        case DRIVE_READBLK: {
            if(!seldrive) break;
            if(moda >= seldrive->blocks) break;

            fseek(seldrive->image, moda * 512, SEEK_SET); // seek to block
            fread(&blockbuffer, 512, 1, seldrive->image);
            drive_info(0, moda);
            break;
        }
        case DRIVE_WRITEBLK: {
            if(!seldrive) break;
            if(moda >= seldrive->blocks) break;
            
            fseek(seldrive->image, moda * 512, SEEK_SET);
            fwrite(&blockbuffer, 512, 1, seldrive->image);
            drive_info(0, moda);
            break;
        }
        case DRIVE_INFO:
            moda = info;
            modb = details;
            break;
        case DRIVE_POLL: {
            if((moda < DRIVES) && (drives[moda].set)) {
                modb = drives[moda].blocks;
                moda = 1;
            } else {
                moda = 0;
                modb = 0;
            }
            break;
        }
        case DRIVE_DOINT:
            dointerrupt = 1;
            break;
        case DRIVE_DONTINT:
            dointerrupt = 0;
            break;
    }
}

static uint32_t drive_readcmd(uint16_t port) {
    return 0;
}

static void drive_writea(uint16_t port, uint32_t value) {
    moda = value;
}

static uint32_t drive_reada(uint16_t port) {
    return moda;
}

static void drive_writeb(uint16_t port, uint32_t value) {
    modb = value;
}

static uint32_t drive_readb(uint16_t port) {
    return modb;
}


void write_blockbuf(uint32_t addr, uint32_t type, uint32_t value) {
    switch(type) {
        case BUS_BYTE:
            write_byte(blockbuffer, addr, value);
            break;
        case BUS_WORD:
            write_word(blockbuffer, addr, value);
            break;
        case BUS_DWORD:
            write_dword(blockbuffer, addr, value);
            break;
    }
}

uint32_t read_blockbuf(uint32_t addr, uint32_t type) {
    switch(type) {
        case BUS_BYTE:
            return read_byte(blockbuffer, addr);
        case BUS_WORD:
            return read_word(blockbuffer, addr);
        case BUS_DWORD:
            return read_dword(blockbuffer, addr);
    }
    return 0;
}

static void drive_handleint(void) {
    uint32_t mode = vm.regs[REG_R10];

    switch(mode) {
        case 0x01: { // load sector at address
            // ax: drive num
            // bx: destination address
            // dx: sector num
            // returns:
            // dx: destination address

            drive_writea(0, vm.regs[REG_AX]); // select drive
            drive_writecmd(0, DRIVE_SELDRIVE); // ^
            drive_writea(0, vm.regs[REG_DX]); // select sector
            drive_writecmd(0, DRIVE_READBLK); // read a sector
            
            for(uint32_t start = 0; start < 512; start++) { // write 512 bytes
                cpu_writebyte(start + vm.regs[REG_BX], blockbuffer[start]);
            }
            break;
        }
    }
}


int drive_attachimage(char *drivepath) {
    drive_t *drive = 0;

    for(int i = 0; i < DRIVES; i++) {
        if(!drives[i].set) { // disk is free
            drive = &drives[i];
            break;
        }
    }

    if(!drive) {
        fprintf(stderr, "drive_attachimage: maximum drives reached\n");
        return 0;
    }

    drive->image = fopen(drivepath, "r+");
    
    if(!drive->image) {
        fprintf(stderr, "drive_attachimage: failed to open drive image (%s)\n", strerror(errno));
        return 0;
    }

    fseek(drive->image, 0, SEEK_END);

    drive->blocks = ftell(drive->image) / 512; // calculate the number of blocks within the image

    drive->set = 1;

    printf("%s loaded as drive %d containing about %d blocks of 512\n", drivepath, drive->id, drive->blocks);

    return 1;
}

void init_drive(void) {
    for(int i = 0; i < DRIVES; i++) {
        if(drives[i].set) {
            fclose(drives[i].image);
            drives[i].id = i;
        }
    }

    vm.ports[0x59].set = 1;
    vm.ports[0x59].write = drive_writecmd;
    vm.ports[0x59].read = drive_readcmd;
    
    vm.ports[0x5A].set = 1;
    vm.ports[0x5A].write = drive_writea;
    vm.ports[0x5A].read = drive_reada;

    vm.ports[0x5B].set = 1;
    vm.ports[0x5B].write = drive_writeb;
    vm.ports[0x5B].read = drive_readb;

    iotableent_t entry = {
        .set = 1,
        .handle = drive_handleint
    };

    iotable.ioentries[0x05] = entry; 
}
