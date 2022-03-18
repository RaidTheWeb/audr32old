#include <pthread.h> // ?
#include <sys/mman.h>
#include <sys/types.h>
#include <unistd.h>

#define THREADED

#include <SDL2/SDL.h>

#include "io.h"
#include "vm.h"

#define WINDOW_WIDTH    640
#define WINDOW_HEIGHT   480

static SDL_Window *window;
static SDL_Renderer *renderer;
static SDL_Texture *texture;

static void screen_destroy(device_t *dev) {
    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}

static uint32_t screen_poll(device_t *dev) {
    return 0;
}

static void screen_pull(device_t *dev, uint32_t data) {
    return;
}

static uint32_t *framebuffer = 0;

void kbd_set_data(device_t *dev, uint8_t scancode);

void io_remove_device(uint16_t id);

void screen_set_title(const char *title) {
    SDL_SetWindowTitle(window, title);
}

static void screen_tick(device_t *dev) {
    for(SDL_Event e; SDL_PollEvent(&e);) {
        switch(e.type) {
            case SDL_KEYDOWN:
            case SDL_KEYUP:
                kbd_set_data(&vm.devices[0x0001], (e.type == SDL_KEYUP ? 0x80 : 0x00) | e.key.keysym.scancode);
                break;
            case SDL_QUIT:
                io_remove_device(dev->id);
                break;
            default:
                break;
        }
    }

    size_t k = 0;
    for(size_t i = 0; i < WINDOW_WIDTH * WINDOW_HEIGHT * 4; i+=4) {
        uint32_t data =  ensurebig32(*(uint32_t *)&vm.memory[ADDR_FRAMEBUFFER + i]);
        if(data) printf("screen: 0x%08x\n", data);
        framebuffer[k++] = data;
    }

    SDL_UpdateTexture(texture, NULL, framebuffer, WINDOW_WIDTH*sizeof(uint32_t));

    SDL_SetRenderDrawColor(renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
    SDL_RenderClear(renderer);
    SDL_RenderCopy(renderer, texture, NULL, NULL);
    SDL_RenderPresent(renderer);
}

//void screen_init(struct Device *dev) {
void screen_init() {
    framebuffer = malloc(WINDOW_WIDTH * WINDOW_HEIGHT * 4);
    /*for(size_t i = 0; i < WINDOW_WIDTH * WINDOW_HEIGHT * 4; i+=4) {
        uint32_t *ptr = (uint32_t *)&vm.memory[ADDR_FRAMEBUFFER + i];
        *ptr = 0x00FF00FF;
    }*/
    struct Device devcopy = {
        .id = (uint16_t)io_request_id(),
        .poll = screen_poll,
        .pull = screen_pull,
        .tick = screen_tick,
        .destroy = screen_destroy
    };

    strncpy(devcopy.name, "dragonfb", sizeof(devcopy.name));

    printf("id: %u, name: %s\n", devcopy.id, devcopy.name);

    SDL_Init(SDL_INIT_VIDEO);
    window = SDL_CreateWindow(
        "VM",
        SDL_WINDOWPOS_UNDEFINED,
        SDL_WINDOWPOS_UNDEFINED,
        WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_SHOWN
    );
    renderer = SDL_CreateRenderer(
        window, -1,  SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC
    );
    texture = SDL_CreateTexture(
        renderer,
        SDL_PIXELFORMAT_RGB888, SDL_TEXTUREACCESS_TARGET,
        WINDOW_WIDTH, WINDOW_HEIGHT
    );
    vm.devices[0x0003] = devcopy;
}
