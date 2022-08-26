#include "std.asm"

#org 0x330000

#define SECOND_OFFSET 0x00330210 ; where we should load the kernel

; second stage for audr32

.word 0x5969 ; boot magic

boot:
    mov r0, msg
    call puts
    mov r10, 0x02
    mov ax, 0x00
    mov bx, SECOND_OFFSET
    mov dx, 0x01
    mov cx, 512 ; an amount worth of sectors
    int 0x05



    jmp SECOND_OFFSET ; jump to high level second stage

jmp boot

printuint:
    mov r10, 0x07
    mov dx, r0
    mov cx, 0
    mov bx, 0x0F00
    int 0x10
    ret

printchar:
    mov r10, 0x01
    mov dx, r0
    mov bx, 0x0F00
    ret

puts:
    puts_print_loop:
        mov r10, 0x01
        cmp [8:r0], 0
        je puts_print_loop_end
        mov dx, [8:r0]
        push r0
        mov bx, 0x0F00
        int 0x10
        pop r0
        add r0, 1
        jmp puts_print_loop
    puts_print_loop_end:
        ret

msg:
    .asciiz "Loading Second Stage!\n"
