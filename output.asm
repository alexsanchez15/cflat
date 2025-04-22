section .rodata
.Lstr0: db "success", 10, 0
len0 equ $ - .Lstr0
.Lstr1: db "double success", 10, 0
len1 equ $ - .Lstr1
.Lstr2: db "failure", 10, 0
len2 equ $ - .Lstr2
.Lstr3: db "terrible", 10, 0
len3 equ $ - .Lstr3
.Lstr4: db "this should print", 10, 0
len4 equ $ - .Lstr4
RESERVED_VARS equ 16
RESERVED_VARS equ 16

section .text
	global _start
_start:
	; code starts here
	mov rbp, rsp
	sub rsp, RESERVED_VARS
	mov rax, 0
	mov qword [rbp - 8], rax
	mov rax, 5
	push rax
	mov rax, 3
	push rax
	mov rax, 2
	mov rbx, rax
	pop rax
	imul rax, rbx
	mov rbx, rax
	pop rax
	add rax, rbx
	mov qword [rbp - 16], rax
	mov rax, 3
	push rax
	mov rax, 2
	push rax
	mov rax, 1
	mov rbx, rax
	pop rax
	add rax, rbx
	pop rsi
	cmp rax, rsi
	sete al
	movzx rax, al
	test rax, rax
jz .end0
	mov rax, .Lstr0
	mov rsi, len0
	mov rdx, rsi	; mov len
	mov rsi, rax	; mov string pointer
	mov rax, 1
	mov rdi, 1
	syscall
	mov rax, [rbp - 16]
	push rax
	mov rax, 11
	pop rsi
	cmp rax, rsi
	sete al
	movzx rax, al
	test rax, rax
jz .end1
	mov rax, .Lstr1
	mov rsi, len1
	mov rdx, rsi	; mov len
	mov rsi, rax	; mov string pointer
	mov rax, 1
	mov rdi, 1
	syscall
.end1:
	mov rax, 100
	push rax
	mov rax, 10
	pop rsi
	cmp rax, rsi
	sete al
	movzx rax, al
	test rax, rax
jz .end2
	mov rax, .Lstr2
	mov rsi, len2
	mov rdx, rsi	; mov len
	mov rsi, rax	; mov string pointer
	mov rax, 1
	mov rdi, 1
	syscall
	mov rax, 1
	push rax
	mov rax, 1
	pop rsi
	cmp rax, rsi
	sete al
	movzx rax, al
	test rax, rax
jz .end3
	mov rax, .Lstr3
	mov rsi, len3
	mov rdx, rsi	; mov len
	mov rsi, rax	; mov string pointer
	mov rax, 1
	mov rdi, 1
	syscall
.end3:
.end2:
	mov rax, 1
	test rax, rax
jz .end4
	mov rax, .Lstr4
	mov rsi, len4
	mov rdx, rsi	; mov len
	mov rsi, rax	; mov string pointer
	mov rax, 1
	mov rdi, 1
	syscall
.end4:
.end0:
	mov rax, [rbp - 16]
	push rax
	mov rax, 60
	pop rdi
	syscall

	;exit cleanly
	mov rax, 60
	mov rdi, 0
	syscall
