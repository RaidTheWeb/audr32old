
;cmp 10, 10
;jz isequal
jmp isequalend

isequal:
    mov ax, 0xFF
isequalend:


cmp 9999999, 9999999
jge isgreater
jmp isgreaterend

isgreater:
    mov bx, 0xEE
isgreaterend:

