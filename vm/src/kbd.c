#include <pthread.h> // ?
#include <string.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <unistd.h>

#include <stdio.h>

#include "interrupts.h"
#include "io.h"
#include "vm.h"

const char ASCIITable[0xFF] = {
    [0x01] = 'a',
    [0x02] = 'b',
    [0x03] = 'c',
    [0x04] = 'd',
    [0x05] = 'e',
    [0x06] = 'f',
    [0x07] = 'g',
    [0x08] = 'h',
    [0x09] = 'i',
    [0x0A] = 'j',
    [0x0B] = 'k',
    [0x0C] = 'l',
    [0x0D] = 'm',
    [0x0E] = 'n',
    [0x0F] = 'o',
    [0x10] = 'p',
    [0x11] = 'q',
    [0x12] = 'r',
    [0x13] = 's',
    [0x14] = 't',
    [0x15] = 'u',
    [0x16] = 'v',
    [0x17] = 'w',
    [0x18] = 'x',
    [0x19] = 'y',
    [0x1A] = 'z',
    [0x1B] = '1',
    [0x1C] = '2',
    [0x1D] = '3',
    [0x1E] = '4',
    [0x1F] = '5',
    [0x20] = '6',
    [0x21] = '7',
    [0x22] = '8',
    [0x23] = '9',
    [0x24] = '0',

    [0x27] = '\b',
    [0x28] = '\t',
    [0x29] = ' ',
    
    [0x2A] = '-',
    [0x2B] = '=',
    [0x2C] = '[',
    [0x2D] = ']',
    [0x2E] = '\\',
    [0x2F] = '\\',

    [0x30] = ';',
    [0x31] = '\'',
    [0x32] = '`',
    [0x33] = ',',
    [0x34] = '.',
    [0x35] = '/',
    
    [0x4C] = '/',
    [0x4D] = '*',
    [0x4E] = '-',
    [0x4F] = '+',
    [0x50] = '\n',
    [0x51] = '1',
    [0x52] = '2',
    [0x53] = '3',
    [0x54] = '4',
    [0x55] = '5',
    [0x56] = '6',
    [0x57] = '7',
    [0x58] = '8',
    [0x59] = '9',
    [0x5A] = '0',
    [0x5B] = '.'
};

static char UpperASCIITable[0xFF] = {
    [0x01] = 'A',
    [0x02] = 'B',
    [0x03] = 'C',
    [0x04] = 'D',
    [0x05] = 'E',
    [0x06] = 'F',
    [0x07] = 'G',
    [0x08] = 'H',
    [0x09] = 'I',
    [0x0A] = 'J',
    [0x0B] = 'K',
    [0x0C] = 'L',
    [0x0D] = 'M',
    [0x0E] = 'N',
    [0x0F] = 'O',
    [0x10] = 'P',
    [0x11] = 'Q',
    [0x12] = 'R',
    [0x13] = 'S',
    [0x14] = 'T',
    [0x15] = 'U',
    [0x16] = 'V',
    [0x17] = 'W',
    [0x18] = 'X',
    [0x19] = 'Y',
    [0x1A] = 'Z',
    [0x1B] = '!',
    [0x1C] = '@',
    [0x1D] = '#',
    [0x1E] = '$',
    [0x1F] = '%',
    [0x20] = '^',
    [0x21] = '&',
    [0x22] = '*',
    [0x23] = '(',
    [0x24] = ')',

    [0x27] = '\b',
    [0x28] = '\t',
    [0x29] = ' ',
    
    [0x2A] = '_',
    [0x2B] = '+',
    [0x2C] = '{',
    [0x2D] = '}',
    [0x2E] = '|',
    [0x2F] = '|',

    [0x30] = ':',
    [0x31] = '"',
    [0x32] = '~',
    [0x33] = '<',
    [0x34] = '>',
    [0x35] = '?',
    
    [0x4C] = '/',
    [0x4D] = '*',
    [0x4E] = '-',
    [0x4F] = '+',
    [0x50] = '\n',
    [0x51] = '1',
    [0x52] = '2',
    [0x53] = '3',
    [0x54] = '4',
    [0x55] = '5',
    [0x56] = '6',
    [0x57] = '7',
    [0x58] = '8',
    [0x59] = '9',
    [0x5A] = '0',
    [0x5B] = '.'
};

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

static uint32_t kbd_poll(uint16_t port) {
    return kbd_buf[kbd_read_i];
}

static uint8_t pressonly = 0;
static uint8_t releaseonly = 1;

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
                        ascii_buf[i] = 0x00; // clear
                    }
                    ascii_write_i = 0;
                    ascii_read_i = 0;
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
                vm.regs[REG_R8] = 0x00;
                vm.regs[REG_R9] = 0x00;
                SET_FLAG(FLAG_ZF, 1);
            } else {
                uint8_t data;
                data = kbd_buf[kbd_read_i]; 
                uint8_t ascii;
                ascii = ascii_buf[ascii_read_i]; 
                vm.regs[REG_R8] = data;
                vm.regs[REG_R9] = ascii;
                SET_FLAG(FLAG_ZF, 0);
            }
            break;
        } 
    } 
} 

static void kbd_pull(uint16_t port, uint32_t data) {
    return;
}

void interrupt_trigger(uint16_t, uint16_t);

void kbd_set_data(device_t *dev, uint8_t scancode) { 
    if(scancode == 0x61 || scancode == 0x65) { uppercase = 1; }
    else if(scancode == (0x61 | 0x80) || scancode == (0x65 | 0x80)) { uppercase = 0; }
    if(scancode > 0x80 && pressonly) return;
    if(scancode < 0x81 && releaseonly) return; 
    kbd_buf[kbd_write_i] = scancode;
    kbd_write_i++;
    kbd_write_i %= 2048;
    ascii_buf[ascii_write_i] = uppercase ? UpperASCIITable[scancode & 0x7F] : ASCIITable[scancode & 0x7F];
    printf("0x%02x, %d, '%c', 0x%02x\n", scancode & 0x7F, uppercase, ascii_buf[ascii_write_i], ascii_buf[ascii_write_i]);
    ascii_write_i++;
    ascii_write_i %= 2048;

    interrupt_trigger(0x01, KBD_INTNUM); // Trigger interrupt on port 0x0001 with the interrupt number 0x0001
    if(scancode > 0x7D) return; 
}

void kbd_init() { 
    iotableent_t keyboardservices = {
        .set = 1,
        .handle = keyboardservices_handleint
    };
    iotable.ioentries[KBD_QUERYNUM] = keyboardservices;

    vm.ports[0x20].set = 1;
    vm.ports[0x20].read = kbd_poll;
    vm.ports[0x20].write = kbd_pull;
}
