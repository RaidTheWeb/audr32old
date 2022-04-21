
main:
    ;mov dx, 'H'
    ;int 0x10
    ;mov dx, 'e'
    ;int 0x10
    ;mov dx, 'l'
    ;int 0x10
    ;mov dx, 'l'
    ;int 0x10
    ;mov dx, 'o'
    ;int 0x10
    ;mov dx, ' '
    ;int 0x10
    ;mov dx, 'W'
    ;int 0x10
    ;mov dx, 'o'
    ;int 0x10
    ;mov dx, 'r'
    ;int 0x10
    ;mov dx, 'l'
    ;int 0x10
    ;mov dx, 'd'
    ;int 0x10
    ;mov dx, '!'
    ;int 0x10
    mov si, HelloWorld ; done did
    mov r10, 0x02 ; string
    mov cx, 12 ; length
    mov bx, 0x0F00 ; colour
    mov r8, 0x00 ; y
    mov r9, 0x00 ; x
    int 0x10
    ; mov r10, 0x03
    ; int 0x10

loop:
    halt
    jmp loop


.data
    HelloWorld:
        .asciiz "Hello World!"
