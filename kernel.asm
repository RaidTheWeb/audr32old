mov bx, KBOOT_STRING
call print_str

jmp Start32_bit

Start32_bit:
    ; register interrupts
    ; outx 0x0002, 0xFFFF0000
    ; outx 0x0002, IDTable
    mov dx, IDTable ; move location of IDT to DX
    int 0x14 ; interrupt controller

    jmp Cmain  

loop: jmp $

    KBOOT_STRING:
    .data
        .ascii "Booting kernel..."
        .byte 0x0A
        .byte 0x00
    SYS_STRING:
        .ascii "Shitty syscall system works!!"
        .byte 0x0A
        .byte 0x00
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

; wanna know what would be hilarious?
; if i just straight up implemented my own syscall system like rn

test_syscall:
    jmp end_data
    .data
        TESTSYSCALL_STRING:
            .ascii "Testing Syscalls!"
            .word 0x0A00
    .text
    end_data:
    mov bx, TESTSYSCALL_STRING
    call print_str
    ret

second_test_syscall:
    jmp end_data2
    .data
        SECONDTESTSYSCALL_STRING:
            .ascii "Hello World!"
            .word 0x0A00
    .text
    end_data2:
    mov bx, SECONDTESTSYSCALL_STRING
    call print_str
    mov r13, 0x00C0FFEE
    ret

third_test_syscall:
    ; prints string
    ; ax: location of string
    mov bx, ax
    call print_str
    ret 

syscall: ; syscall subroutine
    ; dx: syscall number
    push r11
    push bx
    push dx
    mov r11, syscall_table ; add the location of the syscall table to DX meaning DX now points to the start of the table.
    mov bx, 4
    mul bx, dx ; 4 x 2 = 8
    add r11, bx ; syscall_table + 8
    call [32:r11]
    pop dx
    pop bx
    pop r11
    ret

.data
    syscall_table: ; table of syscall numbers in comparison to their kernel calls
        .dword 0x00000000 ; null padding

        .dword test_syscall ; syscall for 0x01
        .dword second_test_syscall ; syscall for 0x02
        .dword third_test_syscall ; syscall for 0x03
        
.text

Cmain: ; if only C actually worked :(
    mov dx, 1
    call syscall
    mov dx, 2
    call syscall
    mov dx, 3
    mov ax, SYS_STRING
    call syscall
    jmp loop 

