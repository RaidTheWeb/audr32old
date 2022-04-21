mov bx, HelloWorld
call print_str
jmp loop

print_str:
    mov r10, 0x01 ; teletype
    cmp [8:bx], 0
    je end

    mov dx, [8:bx]
    push bx ; backup bx
    mov bx, 0x0F01 ; white on black
    int 0x10
    pop bx
    add bx, 1
  
    jmp print_str
end:
    ret

.data

    HelloWorld:
        .asciiz "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
.text

loop:
    jmp loop

