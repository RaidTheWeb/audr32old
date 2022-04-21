;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;; Hello World OS (audr32)                                                         ;;;
;;; ------------------------------------------------------------------------------- ;;;
;;; Quick test to see if I can make a kind of test OS before I create an actual one ;;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

boot:
    call configure_kbd

    mov si, string1 ; banner
    mov r10, 0x02 ; string
    mov bx, 0x0F00 ; colour
    mov r8, 0x00 ; x
    mov r9, 0x00 ; y
    int 0x10

    mov si, string2 ; prompt
    mov r8, 0 ; x
    mov r9, 96 ; y
    int 0x10

    ;mov r10, 0x05 ; set teletype position
    ;mov r8, 0 ; x
    ;mov r9, 128 ; y
    ;int 0x10

check:

    mov r10, 0x01
    int 0x16
    jne done
    jmp check

done:
    mov r10, 0x02
    int 0x16
    mov r10, 0x01 ; char
    mov dx, r9
    int 0x10
    jmp check

hang:
    jmp hang

configure_kbd:
    mov r10, 0x03
    mov ax, 0x02
    mov r8, 1
    int 0x16
    ret

.data

    string2:
        .byte 0xa
        .ascii "Echoing all input below:"
        .byte 0xa
        .byte 0x00 ; null terminate
    
    string1:
        .byte 0xa
        .byte 0x20
        .byte 0x5f
        .byte 0x20
        .byte 0x20
        .byte 0x5f
        .byte 0x20
        .byte 0x20
        .byte 0x20
        .byte 0x20
        .byte 0x20
        .byte 0x5f
        .byte 0x20
        .byte 0x5f
        .byte 0x20
        .byte 0x20
        .byte 0x20
        .byte 0x20
        .byte 0x20
        .byte 0x20
        .byte 0x5f
        .byte 0x5f
        .byte 0x20
        .byte 0x20
        .byte 0x20
        .byte 0x20
        .byte 0x20
        .byte 0x20
        .byte 0x5f
        .byte 0x5f
        .byte 0x20
        .byte 0x20
        .byte 0x20
        .byte 0x20
        .byte 0x20
        .byte 0x20
        .byte 0x20
        .byte 0x5f
        .byte 0x20
        .byte 0x20
        .byte 0x20
        .byte 0x20
        .byte 0x5f
        .byte 0x20
        .byte 0x20
        .byte 0x20
        .byte 0x20
        .byte 0x5f
        .byte 0x5f
        .byte 0x5f
        .byte 0x20
        .byte 0x20
        .byte 0x5f
        .byte 0x5f
        .byte 0x5f
        .byte 0x20
        .byte 0xa
        .byte 0x7c
        .byte 0x20
        .byte 0x7c
        .byte 0x7c
        .byte 0x20
        .byte 0x7c
        .byte 0x5f
        .byte 0x5f
        .byte 0x5f
        .byte 0x7c
        .byte 0x20
        .byte 0x7c
        .byte 0x20
        .byte 0x7c
        .byte 0x5f
        .byte 0x5f
        .byte 0x5f
        .byte 0x20
        .byte 0x20
        .byte 0x5c
        .byte 0x20
        .byte 0x5c
        .byte 0x20
        .byte 0x20
        .byte 0x20
        .byte 0x20
        .byte 0x2f
        .byte 0x20
        .byte 0x2f
        .byte 0x5f
        .byte 0x5f
        .byte 0x20
        .byte 0x5f
        .byte 0x20
        .byte 0x5f
        .byte 0x7c
        .byte 0x20
        .byte 0x7c
        .byte 0x5f
        .byte 0x5f
        .byte 0x7c
        .byte 0x20
        .byte 0x7c
        .byte 0x20
        .byte 0x20
        .byte 0x2f
        .byte 0x20
        .byte 0x5f
        .byte 0x20
        .byte 0x5c
        .byte 0x2f
        .byte 0x20
        .byte 0x5f
        .byte 0x5f
        .byte 0x7c
        .byte 0xa
        .byte 0x7c
        .byte 0x20
        .byte 0x5f
        .byte 0x5f
        .byte 0x20
        .byte 0x2f
        .byte 0x20
        .byte 0x2d
        .byte 0x5f
        .byte 0x29
        .byte 0x20
        .byte 0x7c
        .byte 0x20
        .byte 0x2f
        .byte 0x20
        .byte 0x5f
        .byte 0x20
        .byte 0x5c
        .byte 0x20
        .byte 0x20
        .byte 0x5c
        .byte 0x20
        .byte 0x5c
        .byte 0x2f
        .byte 0x5c
        .byte 0x2f
        .byte 0x20
        .byte 0x2f
        .byte 0x20
        .byte 0x5f
        .byte 0x20
        .byte 0x5c
        .byte 0x20
        .byte 0x27
        .byte 0x5f
        .byte 0x7c
        .byte 0x20
        .byte 0x2f
        .byte 0x20
        .byte 0x5f
        .byte 0x60
        .byte 0x20
        .byte 0x7c
        .byte 0x20
        .byte 0x7c
        .byte 0x20
        .byte 0x28
        .byte 0x5f
        .byte 0x29
        .byte 0x20
        .byte 0x5c
        .byte 0x5f
        .byte 0x5f
        .byte 0x20
        .byte 0x5c
        .byte 0xa
        .byte 0x7c
        .byte 0x5f
        .byte 0x7c
        .byte 0x7c
        .byte 0x5f
        .byte 0x5c
        .byte 0x5f
        .byte 0x5f
        .byte 0x5f
        .byte 0x7c
        .byte 0x5f
        .byte 0x7c
        .byte 0x5f
        .byte 0x5c
        .byte 0x5f
        .byte 0x5f
        .byte 0x5f
        .byte 0x2f
        .byte 0x20
        .byte 0x20
        .byte 0x20
        .byte 0x5c
        .byte 0x5f
        .byte 0x2f
        .byte 0x5c
        .byte 0x5f
        .byte 0x2f
        .byte 0x5c
        .byte 0x5f
        .byte 0x5f
        .byte 0x5f
        .byte 0x2f
        .byte 0x5f
        .byte 0x7c
        .byte 0x20
        .byte 0x7c
        .byte 0x5f
        .byte 0x5c
        .byte 0x5f
        .byte 0x5f
        .byte 0x2c
        .byte 0x5f
        .byte 0x7c
        .byte 0x20
        .byte 0x20
        .byte 0x5c
        .byte 0x5f
        .byte 0x5f
        .byte 0x5f
        .byte 0x2f
        .byte 0x7c
        .byte 0x5f
        .byte 0x5f
        .byte 0x5f
        .byte 0x2f
        .byte 0xa
        .byte 0xa

.text

