#define i 0x33FCCC

mov [8:i], 15
mov r10, 0x33FCCC
; mov dx, [8:r10]
test:
mov dx, test
mov r10, 0x07
mov bx, 0x0F00
int 0x10

jmp $
