#include <pthread.h> // ?
#include <string.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <unistd.h>

#include <stdio.h>

#include "io.h"
#include "vm.h"

#define KBD_PORT 0x0001
#define KBD_INTNUM 0x0001

static uint32_t data;

static uint32_t kbd_poll(device_t *dev) {
    return data;
}

static void kbd_pull(device_t *dev, uint32_t data) {
    return;
}

void interrupt_trigger(uint16_t, uint16_t);

void kbd_set_data(device_t *dev, uint8_t scancode) {
    data = scancode;
    interrupt_trigger(KBD_PORT, KBD_INTNUM); // Trigger interrupt on port 0x0001 with the interrupt number 0x0001
}

void kbd_init() {
    struct Device devcopy = {
        .id = (uint16_t)io_request_id(),
        .poll = kbd_poll,
        .pull = kbd_pull,
        .tick = NULL,
        .destroy = NULL
    };
    strncpy(devcopy.name, "kbd", sizeof(devcopy.name));
    vm.devices[KBD_PORT] = devcopy;
}