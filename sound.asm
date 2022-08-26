; sound test

#include "std.asm" ; include constants

#define AUDIOCMD    0x89
#define AUDIOMODA   0x8A
#define AUDIOMODB   0x8B

sub sp, 0x1000 ; allocate 4KB of memory for our heap at the high end of stack memory (now before the stack as stack grows downwards)

mov ax, 200

loop:
    outx AUDIOMODA, ax
    outx AUDIOMODB, 1000 
    ;outx AUDIOMODB, 220500 ; beep size (should be calculated when i implement the ability to do that (marker calculation?))
    outx AUDIOCMD, 0x03 ; generate wave
    
    outx AUDIOMODA, 0 ; unpause
    outx AUDIOCMD, 0x02 ; play audio

    outx 0x40, 1000
    outx 0x41, 0x0B  

    outx AUDIOCMD, 0x01 ; destroy (we always need to do this after running a sound cycle)
    ; jmp end
    cmp ax, 800
    je end

    add ax, 50
    
    jmp loop

end:
    jmp end


.data
beep:
#incbin "beep.raw" ; include the beep file
.text
