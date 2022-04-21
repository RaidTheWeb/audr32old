jmp main

.data
    #org 1000
    ; .byte 0xE8
    ; .word 0x03E8
    .dword 0x0F0003E8

    #org 0xFFF
    string:
        .string "hello"

.text

#org 0x4000
#define yourmom 'A'

main:
    ; mov [8:1000], yourmom
    mov ax, [32:1000] ; reference dword placed at 1000 (0x000003E80)

    mov [8:1001:-1], 0xFF
    xor [8:1001:-1], 0x02
    mov dx, [8:1001:-1]

    mov [8:1002], 0xFF
    mov bx, [8:1003:-1]

    mov r8, [8:string] ; first character of string
loop:
    halt
    jmp loop
