
; calculate max allocated memory
mov r0, memoryend
mov r1, memory
sub r0, r1

mov cx, memory ; (char *) data pointer (move into start of memory)

mov bx, input

interpretloop:
    inc bx 


    cmp [8:bx], '+'
    je isadd

    cmp [8:bx], '-'
    je issub

    cmp [8:bx], '.'
    je isdot

    cmp [8:bx], ','
    je iscomma

    cmp [8:bx], '>'
    je isgt

    cmp [8:bx], '<'
    je islt

    cmp [8:bx], 0 ; end of input
    je hang
    jmp endswitch


isadd: ; +
    inc [8:cx]
    mov r12, [8:cx]
    jmp endswitch

issub: ; -
    dec [8:cx]
    jmp endswitch

isgt: ; >
    inc r3
    inc cx
    jmp endswitch

islt: ; <
    dec cx
    jmp endswitch

isdot: ; .
    inc r4
    mov r10, 0x01
    mov dx, [8:cx]
    push bx
    mov bx, 0x0F00
    int 0x10
    pop bx
    jmp endswitch

iscomma: ; ,
    inc r5
    outx 0x0002, 0xEEEE0001
    mov r10, 0x02 ; get keystroke
    int 0x16
    mov [8:cx], r9 ; insert ascii character into cx
    jmp endswitch

endswitch:
    jmp interpretloop ; continue to interpret

hang: 
    jmp hang


.data
    input:
        .byte 0 ; pad start to allow for proper loop
        .string "++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++.>+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++<>."
        ;.string ",><."
        .byte 0 ; last pad

    memory:
        .dword 0
        .dword 0
        .dword 0
        .dword 0
        .dword 0
        .dword 0
        .dword 0
        .dword 0
        .dword 0
        .dword 0
        .dword 0
        .dword 0
        .dword 0
        .dword 0
        .dword 0
        .dword 0
        .dword 0
    memoryend: ; use this to calculate max memory
