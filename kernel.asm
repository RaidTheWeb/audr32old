mov bx, KBOOT_STRING
call print_str

jmp Start32_bit

Start32_bit:
    outx 0x0002, 0xEEEE0001 ; await interrupt on keyboard

    ; register interrupts
    outx 0x0002, 0xFFFF0000
    outx 0x0002, IDTable 

    jmp Cmain  

loop: ; i should implement $
    jmp $

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

kbd_handler: ; test interrupt to handle keyboard input
    add r15, 1
    ret

.data ; IDT?!?!?!
    IDTable:
        .byte 1 ; we only have one entry in this table
        .byte 0x01 ; we want to listen for interrupts on the interrupt 0x01
        .dword kbd_handler ; we want it to call kbd_handler as a subroutine whenever a key is pressed, so insert that here
.text



Cmain: ; if only C actually worked :(
   jmp loop 

