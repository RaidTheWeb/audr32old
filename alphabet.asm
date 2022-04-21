mov r10, 0x01
mov bx, 0x0F00
mov r14, 'A'

LOOP:
	cmp r14, 'Z'
	jg END
    
    mov dx, r14
	int 0x10

	add r14, 1

	jmp LOOP

END:
	;nop
    mov dx, 0x0A
	int 0x10
    mov r14, 'A'

;	cmp cx, 40
;	jmp E
	add cx, 1
	halt
	halt
	halt
	halt

	
	jmp LOOP

E:
	;nop
	jmp E

