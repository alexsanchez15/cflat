; assembly to link to on creation of program files
; this is a standard "function" to produce 
; a built in function, this language doesn't really support functions
; takes an integer (in rax), returns pointer to string & its len in rax & rsi
; i have no idea how optimal this code is (it likely isny) but the point is to handwrite
global to_str

section .text
to_str:
    push rbp ; old func ptr
    mov rbp, rsp
    sub rsp, 32 ; reserve 32 bytes here for the string
    xor rcx, rcx ; set to 0s

loop1:
    xor rdx, rdx ; zero out the remainder
    mov rbx, 10
    div rbx ; div rax/10 remainder goes in rdx
    mov r8b, '0'
    add r8b, dl ; lower byte of rdx i believe
    mov [rbp-32 + rcx], r8b
    inc rcx
    test rax, rax
    jnz loop1 ; if rax just holds 0, done with div
    ; now all digits are in reverse order on the stack
    mov rsi, rcx ; want len in rsi after return
    xor rdx, rdx ; 0 out other counter
    dec rcx ; will go over by 1
loop2:
    mov al, [rbp - 32 + rdx] 
    mov bl, [rbp - 32 + rcx]

    mov [rbp - 32 + rdx], bl
    mov [rbp - 32 + rcx], al

    dec rcx
    inc rdx
    cmp rdx, rcx 
    jl loop2
; now string should be in correct order.
    mov byte [rbp - 32 + rsi], 0
    lea rax, [rbp-32]
    mov rsp, rbp ; reset stack
    pop rbp ; reset front of stack ptr
    ret
; just ensure that on calling to_str, 32 btyes are reserved for the string
; (to be safe)
; and if want to store the string (as its on the stack)
; 40 bytes will need to be reserved, 32 for the string, 8 for len int
; so in total, this is a 5 byte operation when results are stored
; (and if they dont need to be stored, still resolve to rax,rsi per my convention)
