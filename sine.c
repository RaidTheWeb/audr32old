#include <SDL2/SDL.h>

uint32_t samplespersec;
uint32_t tonehz;
uint8_t tonevol;
uint32_t sampleindex;
uint32_t squareperiod;
uint32_t halfsquareperiod;
uint32_t bytespersample;

int main(void) {
    samplespersec = 44100;
    tonehz = 800; // Standard Beep Frequency
    tonevol = 120;
    sampleindex = 0;
    squareperiod = samplespersec / tonehz;
    halfsquareperiod = squareperiod / 2;
    bytespersample = sizeof(uint8_t);

    uint32_t bytestowrite = 5*44100; // should be 5 seconds
    uint32_t samples = bytestowrite/bytespersample;

    SDL_Init(SDL_INIT_AUDIO);
    SDL_AudioSpec spec;
    SDL_zero(spec);
    spec.freq = 44100; // 44.1 KHz
    spec.format = AUDIO_U8;
    spec.channels = 1; // mono
    spec.samples = 2048; // 2048 samples
    spec.callback = NULL;

    SDL_AudioDeviceID audio_device = SDL_OpenAudioDevice(NULL, 0, &spec, NULL, 0);

    FILE *f = fopen("beep.raw", "w");

    for(size_t index; index < bytestowrite; index++) {
        uint8_t sample = ((sampleindex++ / halfsquareperiod) % 2) ? tonevol : -tonevol;
        // *buffer++ = sample;
        SDL_QueueAudio(audio_device, &sample, 1);
        fputc(sample, f);
    }
    fclose(f);

    SDL_PauseAudioDevice(audio_device, 0);

    int running = 1;
    while(running) {
        SDL_Event Event;
        while(SDL_PollEvent(&Event)) {
            if(Event.type == SDL_QUIT) {
                running = 0;
                break;
            }
        }
        SDL_Delay(100);
    }

    SDL_CloseAudioDevice(audio_device);
    return 0;
}
