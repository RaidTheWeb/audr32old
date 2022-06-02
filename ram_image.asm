
mov si, string
mov r9, 16
int 0x10

outx 0x10, 'H'
outx 0x10, 'e'
outx 0x10, 'l'
outx 0x10, 'l'
outx 0x10, 'o'
outx 0x10, ' '
outx 0x10, 'W'
outx 0x10, 'o'
outx 0x10, 'r'
outx 0x10, 'l'
outx 0x10, 'd'
outx 0x10, '!'
outx 0x10, 0x0a

outx 0x5A, 0 ; choose drive 0
outx 0x59, 0 ; select drive
outx 0x59, 1 ; read block

mov si, 0x40340000 ; start of block buffer
mov r9, 32
int 0x10 ; (should print hello world)

jmp $

.data

string:
    .asciiz "Currently in RAM!"
