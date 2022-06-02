; AX - Current color
; BX - Screen ptr
; CX - Screen ptr end
; DX - Inner place pixel loop counter
; R8 - Number of inner loops

mov ax, 0
mov bx, 0xB8000

mov cx, 640
mul cx, 480
add cx, 0xB8000

mov dx, 0
mov r8, 1

LOOP:
    cmp bx, cx
    jge RESET

    PLACE_PIXELS:
        mov [bx], ax
        add bx, 1
        add dx, 1

        cmp dx, r8
        je PLACE_PIXELS_END

        jmp PLACE_PIXELS

    PLACE_PIXELS_END:
        mov dx, 0
        add r8, 1

    add ax, 1

    cmp ax, 17
    jne END_AX_RESET

    mov ax, 0

    END_AX_RESET:

    jmp LOOP

RESET:
    mov bx, 0xB8000
    mov r8, 1
    ; mov ax, 0

    jmp LOOP

