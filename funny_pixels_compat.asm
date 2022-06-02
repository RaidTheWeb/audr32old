; BX - Current colour
; R12 - Screen ptr
; R13 - Screen ptr end
; R14 - Num loops
; R15 - Inner pixel loop counter

mov bx, 0x0000 ; black
mov r12, 0xF0000

mov r13, 640
mul r13, 480
add r13, r12

mov r14, 1 ; num loops
mov r15, 0 ; inner place

LOOP:
    cmp r12, r13
    jge RESET

    PLACE_PIXELS:
        mov r10, 0x06
        int 0x10
        ;halt
        mov [32:r12], dx
        add r12, 4
        inc r15

        cmp r15, r14
        je PLACE_PIXELS_END

        jmp PLACE_PIXELS

    PLACE_PIXELS_END:
        mov r15, 0
        inc r14

    inc bx

    cmp bx, 17
    jne END_AX_RESET

    mov bx, 0

    END_AX_RESET:

    jmp LOOP

RESET:
    mov r12, 0xF0000
    mov r14, 1

    jmp LOOP
