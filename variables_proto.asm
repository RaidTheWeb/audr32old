#define a 0xFEEE ; define constant 'a' at the location 0xFEEE

mov [32:a], 5 ; set variable a to 5
mov r10, 0x07
mov dx, [32:a]
mov bx, 0x0F00
int 0x10

jmp $
