#include <pthread.h> // ?
#include <string.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <unistd.h>

#include <stdio.h>

#include "interrupts.h"
#include "io.h"
#include "vm.h"

/*const char ASCIITable[] = {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // Function Keys
    0, '`', 0, 0, 0, 0, 0, 0, 'q', 
    '1', 0, 0, 0, 'z', 's', 'a', 'w',
    '2', 0, 0, 'c', 'x', 'd', 'e', '4',
    '3', 0, 0, ' ', 'v', 'f', 't', 'r',
    '5', 0, 0, 'n', 'b', 'h', 'g', 'y',
    '6', 0, 0, 0, 'm', 'j', 'u', '7',
    '8', 0, 0, ',', 'k', 'i', 'o', '0',
    '9', 0, 0, '.', '/', 'l', ';', 'p',
    '-', 0, 0, 0, '\'', 0, '[', '=', 0,
    0, 0, 0, '\n', ']', 0, '\\', 0, 0, 0,
    0, 0, 0, 0, 0, '\b', 0, 0, '1', 0, '4',
    '7', 0, 0, 0, '0', '.', '2', '5', '6',
    '8', 0, 0, 0, '+', '3', '-', '*', '9',
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, '/', 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, '\n', // 0...

};*/

const char ASCIITable[] = {
    [0x0E] = '`',
    [0x15] = 'q',
    [0x16] = '1',
    [0x1A] = 'z',
    [0x1B] = 's',
    [0x1C] = 'a',
    [0x1D] = 'w',
    [0x1E] = '2',
    [0x21] = 'c',
    [0x22] = 'x',
    [0x23] = 'd',
    [0x24] = 'e',
    [0x25] = '4',
    [0x26] = '3',
    [0x29] = ' ',
    [0x2A] = 'v',
    [0x2B] = 'f',
    [0x2C] = 't',
    [0x2D] = 'r',
    [0x2E] = '5',
    [0x31] = 'n',
    [0x32] = 'b',
    [0x33] = 'h',
    [0x34] = 'g',
    [0x35] = 'y',
    [0x36] = '6',
    [0x3A] = 'm',
    [0x3B] = 'j',
    [0x3C] = 'u',
    [0x3D] = '7',
    [0x3E] = '8',
    [0x41] = ',',
    [0x42] = 'k',
    [0x43] = 'i',
    [0x44] = 'o',
    [0x45] = '0',
    [0x46] = '9',
    [0x49] = '.',
    [0x4A] = '/',
    [0x4B] = 'l',
    [0x4C] = ';',
    [0x4D] = 'p',
    [0x4E] = '-',
    [0x52] = '\'',
    [0x54] = '[',
    [0x55] = '=',
    [0x5A] = '\n',
    [0x5B] = ']',
    [0x5D] = '\\',
    [0x66] = '\b',
    [0x69] = '1',
    [0x6B] = '4',
    [0x6C] = '7',
    [0x70] = '0',
    [0x71] = '.',
    [0x72] = '2',
    [0x73] = '5',
    [0x74] = '6',
    [0x75] = '8',
    [0x79] = '+',
    [0x7A] = '3',
    [0x7B] = '-',
    [0x7C] = '*',
    [0x7D] = '9',
};

static char UpperASCIITable[] = {
    [0x0E] = '~',
    [0x15] = 'Q',
    [0x16] = '!',
    [0x1A] = 'Z',
    [0x1B] = 'S',
    [0x1C] = 'A',
    [0x1D] = 'W',
    [0x1E] = '@',
    [0x21] = 'C',
    [0x22] = 'X',
    [0x23] = 'D',
    [0x24] = 'E',
    [0x25] = '$',
    [0x26] = '#',
    [0x29] = ' ',
    [0x2A] = 'V',
    [0x2B] = 'F',
    [0x2C] = 'T',
    [0x2D] = 'R',
    [0x2E] = '%',
    [0x31] = 'N',
    [0x32] = 'B',
    [0x33] = 'H',
    [0x34] = 'G',
    [0x35] = 'Y',
    [0x36] = '^',
    [0x3A] = 'M',
    [0x3B] = 'J',
    [0x3C] = 'U',
    [0x3D] = '&',
    [0x3E] = '*',
    [0x41] = '<',
    [0x42] = 'K',
    [0x43] = 'I',
    [0x44] = 'O',
    [0x45] = ')',
    [0x46] = '(',
    [0x49] = '>',
    [0x4A] = '?',
    [0x4B] = 'L',
    [0x4C] = ':',
    [0x4D] = 'P',
    [0x4E] = '_',
    [0x52] = '"',
    [0x54] = '{',
    [0x55] = '+',
    [0x5A] = '\n',
    [0x5B] = '}',
    [0x5D] = '|',
    [0x66] = '\b',

};

#define KBD_PORT 0x0001
#define KBD_INTNUM 0x0001
#define KBD_QUERYNUM 0x0016

static int currentmode = 0;
static int uppercase = 0;

static size_t kbd_write_i = 0;
static size_t kbd_read_i = 0;
static char kbd_buf[2048];

static size_t ascii_write_i = 0;
static size_t ascii_read_i = 0;
static char ascii_buf[2048];

static uint32_t kbd_poll(device_t *dev) {
    return kbd_buf[kbd_read_i];
}

static uint8_t pressonly;
static uint8_t releaseonly;

static void keyboardservices_handleint() {
    uint8_t mode = vm.regs[REG_R10];

    switch(mode) {
        case 0x03: { // CONFIG
            uint8_t mode = vm.regs[REG_AX];

            switch(mode) {
                case 0x03: { // ONLY ACCEPT RELEASE
                    // r8: enable
                    releaseonly = vm.regs[REG_R8];
                    pressonly = 0;
                    break;
                }
                case 0x02: { // ONLY ACCEPT PRESS
                    // r8: enable
                    pressonly = vm.regs[REG_R8];
                    releaseonly = 0;
                    break;
                }
                case 0x01: { // CLEAR BUFFER
                    for(int i = 0; i < 2048; i++) {
                        kbd_buf[i] = 0x00; // clear
                    }
                    kbd_write_i = 0;
                    kbd_read_i = 0;
                    break;
                }
            }
            break;
        }
        case 0x02: { // GET KEYSTROKE
            // returns:
            // r8: scancode
            // r9: ascii character
            
            if(kbd_read_i == kbd_write_i) {
                vm.regs[REG_R8] = 0x00;
                vm.regs[REG_R9] = 0x00;
            } else {
                uint8_t data;
                data = kbd_buf[kbd_read_i];
                kbd_read_i++;
                kbd_read_i %= 2048;
                uint8_t ascii;
                ascii = ascii_buf[ascii_read_i];
                ascii_read_i++;
                ascii_read_i %= 2048;

                vm.regs[REG_R8] = data;
                vm.regs[REG_R9] = ascii;
            }
            break;
        }
        case 0x01: { // CHECK KEYSTROKE
            // returns:
            // r8: scancode
            // r9: ascii character
            // zf(flag): available (0 if yes, 1 if no)
            uint8_t data = 0;
            if(kbd_read_i == kbd_write_i) {
                printf("data is not available, setting zero flag and clearing registers\n");
                vm.regs[REG_R8] = 0x00;
                vm.regs[REG_R9] = 0x00;
                vm.flags[FLAG_ZF] = 1;
                printf("done setting registers and zero flag\n");

            } else {
                printf("data is available, clearing zero flag and setting registers\n");
                uint8_t data;
                data = kbd_buf[kbd_read_i]; 
                uint8_t ascii;
                ascii = ascii_buf[ascii_read_i]; 

                vm.regs[REG_R8] = data;
                vm.regs[REG_R9] = ascii;
                vm.flags[FLAG_ZF] = 0;
            }
            break;
        } 
    }
    printf("finished handling keyboard services interrupt!\n");
} 

static void kbd_pull(device_t *dev, uint32_t data) {
    return;
}

void interrupt_trigger(uint16_t, uint16_t);

void kbd_set_data(device_t *dev, uint8_t scancode) {
    if(scancode == 0x12 || scancode == 0x59) { uppercase = 1; }
    else if(scancode == (0x12 | 0x80) || scancode == (0x59 | 0x80)) { uppercase = 0; }
    if(scancode > 0x7D && pressonly) return;
    if(scancode < 0x7D && releaseonly) return;
    kbd_buf[kbd_write_i] = scancode;
    kbd_write_i++;
    kbd_write_i %= 2048;
    ascii_buf[ascii_write_i] = uppercase ? UpperASCIITable[scancode & 0x7F] : ASCIITable[scancode & 0x7F];
    ascii_write_i++;
    ascii_write_i %= 2048;

    interrupt_trigger(KBD_PORT, KBD_INTNUM); // Trigger interrupt on port 0x0001 with the interrupt number 0x0001
    if(scancode > 0x7D) return;
    printf("keyboard 0x%02x %c\n", scancode, ASCIITable[scancode]);
}

static void kbd_tick(device_t *dev) {
    //printf("ticking keyboard!\n");
}

void kbd_init() {
    struct Device devcopy = {
        .id = (uint16_t)io_request_id(),
        .set = 1,
        .poll = kbd_poll,
        .pull = kbd_pull,
        .tick = kbd_tick,
        .destroy = NULL
    };
    strncpy(devcopy.name, "kbd", sizeof(devcopy.name));
    
    iotableent_t keyboardservices = {
        .set = 1,
        .handle = keyboardservices_handleint
    };
    iotable.ioentries[KBD_QUERYNUM] = keyboardservices;

    vm.devices[KBD_PORT] = devcopy;
}
