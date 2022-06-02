
mov r10, 0x02
mov si, string 
mov bx, 0x0F00
int 0x10

jmp 0x330000 ; make the jump 

.data

string:
    .asciiz "Booting into RAM image"
