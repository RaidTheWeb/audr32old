mov r8, 15
mov [8:0xFBBB], r8
lea r8, [32:0xFBBB]

mov [32:0xFBBC], r8
mov r8, [32:0xFBBC]
mov r8, [32:r8]


jmp $
