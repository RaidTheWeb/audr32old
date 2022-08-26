#include <SDL2/SDL.h>

#include "io.h"
#include "vm.h"

#define SAMPLERATE 44100 // 44.1 KHz
#define CHANNELS 1 // mono
#define SAMPLES 2048 // 2048 samples
#define FORMAT AUDIO_U8

uint32_t sampleindex = 0;


enum {
    AUDIO_LOAD, 
    AUDIO_DESTROY,
    AUDIO_PAUSE,
    AUDIO_LOADBEEP
};

uint32_t audiomoda = 0;
uint32_t audiomodb = 0;

uint8_t audiobufferset = 0;
SDL_AudioDeviceID audiodev;

static void audio_writecmd(uint16_t port, uint32_t data) { 
    switch(data) {
        case AUDIO_LOAD: { // Load audio buffer

            // moda: location
            // modb: length
            // logic here 
            if(audiomodb == 0) return; // length cannot be zero

            SDL_AudioSpec spec;
            spec.freq = SAMPLERATE;
            spec.channels = CHANNELS; // mono
            spec.samples = SAMPLES; // 2048 sample buffer size
            spec.format = FORMAT; // unsigned 8-bits
            spec.callback = NULL;
            spec.userdata = NULL;

            audiodev = SDL_OpenAudioDevice(NULL, 0, &spec, NULL, 0);
            audiobufferset = 1;

            for(size_t index = 0; index < audiomodb; index++) {
                uint8_t byte = cpu_readbyte(audiomoda + index);
                SDL_QueueAudio(audiodev, &byte, 1); // queue byte 
            }
            break;
        }
        case AUDIO_LOADBEEP: { // Generate square wave beep
            // moda: frequency
            // modb: duration (ms)

            if(audiomoda == 0 || audiomodb == 0) return;

            SDL_AudioSpec spec;
            spec.freq = SAMPLERATE;
            spec.channels = CHANNELS; // mono
            spec.samples = SAMPLES; // 2048 sample buffer size
            spec.format = FORMAT; // unsigned 8-bits
            spec.callback = NULL;
            spec.userdata = NULL;

            audiodev = SDL_OpenAudioDevice(NULL, 0, &spec, NULL, 0);
            audiobufferset = 1;

            uint32_t sampleindex = 0;
            uint32_t squareperiod = SAMPLERATE / audiomoda;
            uint32_t halfsquareperiod = squareperiod / 2;
            for(size_t index = 0; index < ((SAMPLERATE / 1000.0) * audiomodb); index++) {
                uint8_t sample = ((sampleindex++ / halfsquareperiod) % 2) ? 120 : -120;
                SDL_QueueAudio(audiodev, &sample, 1);
            }
            break;
        }
        case AUDIO_PAUSE: { // pause and play audio
            // moda: pause state
            if(audiomoda) SDL_PauseAudioDevice(audiodev, 1);
            else {
                if(audiobufferset) {
                    SDL_PauseAudioDevice(audiodev, 0);
                } else
                    return;
            }
            break;
        }
        case AUDIO_DESTROY: { // destroy audio instance
            SDL_CloseAudioDevice(audiodev);
            audiobufferset = 0;
            break;
        }
    }
}

static uint32_t audio_readcmd(uint16_t port) {
    return 0;
}

static void audio_writemoda(uint16_t port, uint32_t data) {
    audiomoda = data;
}

static uint32_t audio_readmoda(uint16_t port) {
    return audiomoda;
}

static void audio_writemodb(uint16_t port, uint32_t data) {
    audiomodb = data;
}

static uint32_t audio_readmodb(uint16_t port) {
    return audiomodb;
}

void audio_init(void) {

    vm.ports[0x89].set = 1;
    vm.ports[0x89].write = audio_writecmd;
    vm.ports[0x89].read = audio_readcmd;

    vm.ports[0x8A].set = 1;
    vm.ports[0x8A].write = audio_writemoda;
    vm.ports[0x8A].read = audio_readmoda;

    vm.ports[0x8B].set = 1;
    vm.ports[0x8B].write = audio_writemodb;
    vm.ports[0x8B].read = audio_readmodb;
}
