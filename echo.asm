
boot:
    call configurekbd

check:
    mov r10, 0x01
    int 0x16
    jne done
    jmp check

done:
    mov r10, 0x02
    int 0x16
    mov r10, 0x01
    mov bx, 0x0F00
    mov dx, r9
    int 0x10
    jmp check

hang:
    jmp hang

configurekbd:
    mov r10, 0x03
    mov ax, 0x02
    mov r8, 0x01
    int 0x16
    ret
