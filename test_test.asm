
call main
jmp $

printint:
	push bp
	mov bp, sp
	sub sp, 4
	mov r10, 0x07
	mov dx, di
	mov bx, 0x0F00
	int 0x10
	add sp, 4
	pop bp
	ret

main:
; perhaps because bp is 0?
	push bp
	mov bp, sp
	sub sp, 16
	mov [32:bp:-12], 10
	mov [32:bp:-8], 20
	mov [32:bp:-4], 30
	mov di, [32:bp:-4]
	call printint
	mov di, [32:bp:-8]
	call printint
	mov di, [32:bp:-12]
	call printint
	add sp, 16
	pop bp
	ret
