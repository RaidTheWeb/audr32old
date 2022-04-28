#include <errno.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <unistd.h>

#define THREADED


#ifdef THREADED
    #include <pthread.h>
#endif

#include <SDL2/SDL.h>

#include "bitmap_font.h"
#include "interrupts.h"
#include "io.h"
#include "vm.h"

#ifdef THREADED
    static pthread_t screen_blit_th;
#endif

static uint8_t keymap[SDL_NUM_SCANCODES] = {
    [SDL_SCANCODE_A] = 0x1C,
    [SDL_SCANCODE_B] = 0x32,
    [SDL_SCANCODE_C] = 0x21,
    [SDL_SCANCODE_D] = 0x23,
    [SDL_SCANCODE_E] = 0x24,
    [SDL_SCANCODE_F] = 0x2B,
    [SDL_SCANCODE_G] = 0x34,
    [SDL_SCANCODE_H] = 0x33,
    [SDL_SCANCODE_I] = 0x43,
    [SDL_SCANCODE_J] = 0x3B,
    [SDL_SCANCODE_K] = 0x42,
    [SDL_SCANCODE_L] = 0x4B,
    [SDL_SCANCODE_M] = 0x3A,
    [SDL_SCANCODE_N] = 0x31,
    [SDL_SCANCODE_O] = 0x44,
    [SDL_SCANCODE_P] = 0x4D,
    [SDL_SCANCODE_Q] = 0x15,
    [SDL_SCANCODE_R] = 0x2D,
    [SDL_SCANCODE_S] = 0x1B,
    [SDL_SCANCODE_T] = 0x2C,
    [SDL_SCANCODE_U] = 0x3C,
    [SDL_SCANCODE_V] = 0x2A,
    [SDL_SCANCODE_W] = 0x1D,
    [SDL_SCANCODE_X] = 0x22,
    [SDL_SCANCODE_Y] = 0x35,
    [SDL_SCANCODE_Z] = 0x1A,

    [SDL_SCANCODE_1] = 0x16,
    [SDL_SCANCODE_2] = 0x1E,
    [SDL_SCANCODE_3] = 0x26,
    [SDL_SCANCODE_4] = 0x25,
    [SDL_SCANCODE_5] = 0x2E,
    [SDL_SCANCODE_6] = 0x36,
    [SDL_SCANCODE_7] = 0x3D,
    [SDL_SCANCODE_8] = 0x3E,
    [SDL_SCANCODE_9] = 0x46,
    [SDL_SCANCODE_0] = 0x45,

    [SDL_SCANCODE_RETURN] = 0x5A,
    [SDL_SCANCODE_ESCAPE] = 0x76,
    [SDL_SCANCODE_BACKSPACE] = 0x66,
    [SDL_SCANCODE_TAB] = 0x0D,
    [SDL_SCANCODE_SPACE] = 0x29,

    [SDL_SCANCODE_MINUS] = 0x4E,
    [SDL_SCANCODE_EQUALS] = 0x55,
    [SDL_SCANCODE_LEFTBRACKET] = 0x54,
    [SDL_SCANCODE_RIGHTBRACKET] = 0x5B,
    [SDL_SCANCODE_BACKSLASH] = 0x5D,
    [SDL_SCANCODE_NONUSBACKSLASH] = 0x5D,

    [SDL_SCANCODE_SEMICOLON] = 0x4C,
    [SDL_SCANCODE_APOSTROPHE] = 0x52,
    [SDL_SCANCODE_GRAVE] = 0x0E,
    [SDL_SCANCODE_COMMA] = 0x41,
    [SDL_SCANCODE_PERIOD] = 0x29,
    [SDL_SCANCODE_SLASH] = 0x4A,

    [SDL_SCANCODE_F1] = 0x05,
    [SDL_SCANCODE_F2] = 0x06,
    [SDL_SCANCODE_F3] = 0x04,
    [SDL_SCANCODE_F4] = 0x0C,
    [SDL_SCANCODE_F5] = 0x03,
    [SDL_SCANCODE_F6] = 0x0B,
    [SDL_SCANCODE_F7] = 0x83,
    [SDL_SCANCODE_F8] = 0x0A,
    [SDL_SCANCODE_F9] = 0x01,
    [SDL_SCANCODE_F10] = 0x09,
    [SDL_SCANCODE_F11] = 0x78,
    [SDL_SCANCODE_F12] = 0x07,

    [SDL_SCANCODE_INSERT] = 0x70,
    [SDL_SCANCODE_HOME] = 0x6C,
    [SDL_SCANCODE_PAGEUP] = 0x7D,
    [SDL_SCANCODE_DELETE] = 0x71,
    [SDL_SCANCODE_END] = 0x69,
    [SDL_SCANCODE_PAGEDOWN] = 0x7A,
    [SDL_SCANCODE_RIGHT] = 0x74,
    [SDL_SCANCODE_LEFT] = 0x6B,
    [SDL_SCANCODE_DOWN] = 0x72,
    [SDL_SCANCODE_UP] = 0x75,

    [SDL_SCANCODE_KP_DIVIDE]   = 0x4A,
    [SDL_SCANCODE_KP_MULTIPLY] = 0x7C,
    [SDL_SCANCODE_KP_MINUS]    = 0x7B,
    [SDL_SCANCODE_KP_PLUS]     = 0x79,
    [SDL_SCANCODE_KP_ENTER]    = 0x5A,
    [SDL_SCANCODE_KP_1]        = 0x69,
    [SDL_SCANCODE_KP_2]        = 0x72,
    [SDL_SCANCODE_KP_3]        = 0x7A,
    [SDL_SCANCODE_KP_4]        = 0x6B,
    [SDL_SCANCODE_KP_5]        = 0x73,
    [SDL_SCANCODE_KP_6]        = 0x74,
    [SDL_SCANCODE_KP_7]        = 0x6C,
    [SDL_SCANCODE_KP_8]        = 0x75,
    [SDL_SCANCODE_KP_9]        = 0x7D,
    [SDL_SCANCODE_KP_0]        = 0x70,
    [SDL_SCANCODE_KP_PERIOD]   = 0x71,

    [SDL_SCANCODE_APPLICATION]    = 0x2F,

    [SDL_SCANCODE_LCTRL]  = 0x14,
    [SDL_SCANCODE_LSHIFT] = 0x12,
    [SDL_SCANCODE_LALT]   = 0x11,
    [SDL_SCANCODE_LGUI]   = 0x1F,
    [SDL_SCANCODE_RCTRL]  = 0x14,
    [SDL_SCANCODE_RSHIFT] = 0x59,
    [SDL_SCANCODE_RALT]   = 0x11,
    [SDL_SCANCODE_RGUI]   = 0x27,
};

#define VIDEOSERVICE 0x10

#define WINDOW_WIDTH    640
#define WINDOW_HEIGHT   480

static int stop = 0;
static SDL_Window *window;
static SDL_Renderer *renderer;
static SDL_Texture *texture;

static void screen_destroy(device_t *dev) {
    stop = 1;
    pthread_join(screen_blit_th, NULL);
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
    //printf("ticking screen!\n");
    for(SDL_Event e; SDL_PollEvent(&e);) {
        switch(e.type) {
            case SDL_KEYDOWN:
            case SDL_KEYUP:
                // (e.type == SDL_KEYUP ? 0x80 : 0x00) 
                kbd_set_data(&vm.devices[0x0001], (e.type == SDL_KEYUP ? 0x80 : 0x00) | keymap[e.key.keysym.scancode]);
                break;
            case SDL_QUIT:
                io_remove_device(dev->id);
                break;
            default:
                break;
        }
    }
    
    //printf("finished screen tick routine.\n");
}

void screen_blit(device_t *dev) {
    size_t k = 0;
    for(size_t i = 0; i < WINDOW_WIDTH * WINDOW_HEIGHT * 4; i+=4) { 
        ptr_t pointer = {
            .addr = ADDR_FRAMEBUFFER + i,
            .ptrmode = 0x03, // 32 bit
        };  
        uint32_t data = GET_PTR(pointer);
        framebuffer[k++] = data;
    }

    SDL_UpdateTexture(texture, NULL, framebuffer, WINDOW_WIDTH*sizeof(uint32_t));

    SDL_SetRenderDrawColor(renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
    SDL_RenderClear(renderer);
    SDL_RenderCopy(renderer, texture, NULL, NULL);
    SDL_RenderPresent(renderer);
}

static void setpixel(uint32_t x, uint32_t y, uint32_t colour) {
    uint32_t index = x * (32/8) +
        y * (WINDOW_WIDTH*(32/8));
    ptr_t pointer = {
        .addr = ADDR_FRAMEBUFFER + index,
        .ptrmode = 0x03
    };
    //printf("drawing pixel 0x%08x 0x%08x\n", x, y);
    SET_PTR(pointer, colour);
}

static void drawrect(uint32_t offsetx, uint32_t offsety, uint32_t w, uint32_t h, uint32_t colour) {
    for(uint32_t y = offsety; y <= (offsety + h); y++) {
        for(uint32_t x = offsetx; x <= (offsetx + w); x++) {
            setpixel(x, y, colour);
        }
    }
}

static void drawcircle(uint32_t offsetx, uint32_t offsety, uint32_t r, uint32_t colour) {
    int radiusSquared = r * r;
	
	int minX = ((offsetx >= r) ? (offsetx -r) : 0), maxX = (offsetx + r);
	if(maxX > WINDOW_WIDTH)
		maxX = WINDOW_WIDTH - 1;
	int minY = ((offsety >= r) ? (offsety -r) : 0), maxY = (offsety +r);
	if(maxY > WINDOW_HEIGHT)
		maxY = WINDOW_HEIGHT -1;
	
	for(int currY = minY; currY <= maxY; currY++)
	{
		for(int currX = minX; currX <= maxX; currX++)
		{
			if(radiusSquared > ((currX-offsetx)*(currX-offsetx)+(currY-offsety)*(currY-offsety)))
				setpixel( currX, currY, colour);
		}
	}
}

#define bit_test(var, offset) ({ \
    int __ret; \
    __ret = (var >> (offset)); \
    __ret & 1; \
})

static uint8_t *font;

static void drawchar(char c, int x, int y, uint32_t hex_fg, uint32_t hex_bg) {
    int orig_x = x;
    uint8_t *glyph = &font[c * bitmap_font_height];

    for (int i = 0; i < bitmap_font_height; i++) {
        for (int j = bitmap_font_width - 1; j >= 0; j--)
            setpixel(x++, y, bit_test(glyph[i], j) ? hex_fg : hex_bg);
        y++;
        x = orig_x;
    } 

    return;
}

static uint32_t colours[0xF + 1] = {
    0x00000000, // black
    0x000000DD, // blue
    0x0000DD00, // green
    0x0000DDDD, // cyan
    0x00DD0000, // red
    0x00DD00DD, // magenta
    0x00DDDD00, // brown
    0x009B9B9B, // light grey
    0x004A4A4A, // dark grey
    0x000000FF, // blue
    0x0000FF00, // green
    0x0000FFFF, // cyan
    0x00FF0000, // red
    0x00FF00FF, // magenta
    0x00FFFF00, // brown
    0x00FFFFFF, // white
};

static int32_t charx = -bitmap_font_width;
static int32_t chary = 0;

static uint32_t parsefg(uint32_t colour) {
    int fg = (colour & 0x0000FF00) >> 8;
    return colours[fg];
}

static uint32_t parsebg(uint32_t colour) {
    int bg = (colour & 0x000000FF);
    return colours[bg];
}

static void videoservice_handleint(void) {
    uint8_t mode = vm.regs[REG_R10];
    switch(mode) {
        case 0x05: { // SET TELETYPE POS
            // r8: column
            // r9: row
            charx = vm.regs[REG_R8];
            chary = vm.regs[REG_R9];
            break;
        }
        case 0x04: { // WRITE PIXEL
            // bx: colour index
            // r8: column
            // r9: row
            setpixel(vm.regs[REG_R8], vm.regs[REG_R9], parsebg(vm.regs[REG_BX]));
            break;
        }
        case 0x03: { // CLEAR
            // bx: colour
            drawrect(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, parsebg(vm.regs[REG_BX]));
            break;
        }
        case 0x02: { // WRITE STRING
            // si: source address of string
            // cx: string length (can be zero to go until null terminated)
            // bx: colour index
            // r8: column
            // r9: row
            uint32_t address = vm.regs[REG_SI]; // string source
            if(vm.regs[REG_CX] != 0) { // length is set
                size_t cursor = 0;
                uint32_t x = 0;
                uint32_t y = 0;
                uint8_t byte = 0;
                for(size_t i = 0; i < vm.regs[REG_CX]; i++) {
                    byte = vm.memory[address + cursor++];
                    if(byte == 0x0A) {
                        x = 0;
                        y += bitmap_font_height;
                        continue;
                    }
                    drawchar(byte, x + vm.regs[REG_R8], y + vm.regs[REG_R9], parsefg(vm.regs[REG_BX]), parsebg(vm.regs[REG_BX]));
                    x += bitmap_font_width;
                } 
                charx = x + vm.regs[REG_R8] > 0 ? x + vm.regs[REG_R8] : -bitmap_font_width;
                chary = y + vm.regs[REG_R9];

            } else {
                size_t cursor = 0;
                uint32_t x = 0;
                uint32_t y = 0;
                uint8_t byte = 0;
                while((byte = vm.memory[address + cursor++]) != 0) {
                    if(byte == 0x0A) {
                        x = 0;
                        y += bitmap_font_height;
                        continue;
                    }
                    drawchar(byte, x + vm.regs[REG_R8], y + vm.regs[REG_R9], parsefg(vm.regs[REG_BX]), parsebg(vm.regs[REG_BX]));
                    x += bitmap_font_width;
                }
                charx = x + vm.regs[REG_R8] > 0 ? x + vm.regs[REG_R8] : -bitmap_font_width;
                chary = y + vm.regs[REG_R9];
            }
            break;
        }

        case 0x01:
        default: { // DRAW CHARACTER (TELETYPE, AUTOMATIC HANDLING OF SPECIAL TELETYPE INTERFACE FEATURES)
            // dx: character
            // bx: colour
           
            char c = vm.regs[REG_DX];
            if(charx >= WINDOW_WIDTH || c == 0x0A) {
                chary += bitmap_font_height;
                charx = -bitmap_font_width;
                if(c == 0x0A) return;
            }
            if(chary >= WINDOW_HEIGHT - bitmap_font_height) {
                drawrect(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, parsebg(0x0000)); // clear screen
                chary = 0;
                charx = -bitmap_font_width;
            } else if(c == 0x08) {
                drawrect(charx, chary, bitmap_font_width, bitmap_font_height, parsebg(vm.regs[REG_BX]));
                charx -= bitmap_font_width;
                if(charx < -bitmap_font_width) {
                    charx = -bitmap_font_width;
                }
                return;
            } 
            drawchar(c, charx += bitmap_font_width, chary, parsefg(vm.regs[REG_BX]), parsebg(vm.regs[REG_BX])); 
            break;
        }
    } 
}

void screen_init() {
    framebuffer = malloc(WINDOW_WIDTH * WINDOW_HEIGHT * 4);
    
    struct Device devcopy = {
        .id = (uint16_t)io_request_id(),
        .set = 1,
        .poll = screen_poll,
        .pull = screen_pull,
        .tick = screen_tick,
        .destroy = screen_destroy
    };

    strncpy(devcopy.name, "dragonfb", sizeof(devcopy.name));
 
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
    pthread_create(&screen_blit_th, NULL, (void *(*)(void *))screen_blit, &vm.devices[0x0003]); // initial blit (ensure non-blocking)

    drawrect(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, parsebg(0x0000));
    font = bitmap_font; 

    vm.devices[0x0003] = devcopy;

    iotableent_t videoservice = {
        .set = 1,
        .handle = videoservice_handleint
    };
    iotable.ioentries[VIDEOSERVICE] = videoservice;
}
