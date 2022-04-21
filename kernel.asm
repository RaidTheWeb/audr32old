mov bx, KBOOT_STRING
call print_str

jmp Start32_bit

Start32_bit:
    jmp Cmain  

loop: ; i should implement $
    jmp loop

.data
    KBOOT_STRING:
        .asciiz "Booting kernel"

.text


print_str:
    mov r10, 0x01 ; teletype
    cmp [8:bx], 0
    je end

    mov dx, [8:bx]
    push bx ; backup bx
    mov bx, 0x0F00 ; white on black
    int 0x10
    pop bx
    add bx, 1
  
    jmp print_str
end:
    ret






Cmain: ; if only C actually worked :(
    
