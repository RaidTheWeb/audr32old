
yourmother:
    mov di, [ax] ; reference to 32 bit pointer address location using AX
main:
    mov dx, 0xFFFF ; hexadecimal
    mov ax, 61166 ; integers
    mov bx, 0o207 ; octals


    mov cx, 0b10101010 ; binary

    mov bp, cx ; registers

    mov si, [32:0x000] ; pointers

    mov [32:0x00FF], 0xDEADBEEF
    ;mov ip, yourmother ; set instruction pointer via code
    mov [32:0x00BB], [32:0x00FF]
    mov r9, [32:0x00BB]
loop:
    halt
    jnz loop, r9 ; loop halt

