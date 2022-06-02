
push bp
mov bp, sp
sub sp, 16
mov [32:bp:-4], 2
mov [32:bp:-8], 4

mov ax, [32:bp:-4]
