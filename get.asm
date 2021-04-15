    default rel
    extern hashing_function
    global get
 
section .text

; DESTRUCT : RAX, RBX, RSI, RDI
mstrcmp:
    ;rsi = [string1]
    ;rdi =[string2]

cmp_loop:

    mov rax, [rsi] ; rsi
    mov rbx, [rdi] ; rdi
    
    add rsi, 8
    add rdi, 8

    sub rax, rbx
    cmp rax, 0
    jne return_cmp

    mov al, BYTE [rsi]
    mov bl, BYTE [rdi]

    cmp al, 0
    jne rsi_not_zero
    movzx rax, bl
    ret

rsi_not_zero:
    cmp bl, 0
    jne cmp_loop
    movzx rax, al
    ret

return_cmp:
    ret

; rdi = hash_table ptr
; rsi = const char* key
get:
    mov r8, rdi ; r8 = hash_table ptr
    mov r9, rsi ; r9 = char ptr

    mov rdi, r9
    call hashing_function ; rax = hash

    mov rcx, [r8] ; rcx = capacity
    xor rdx, rdx

    div rcx 
    mov rax, rdx
    shl rax, 0x6
    add rax, QWORD [r8 + 0x10] ; rax = curr bucket
    
    mov r10, rax               ; r10 = curr bucket
    mov r11, [r10 + 0x18]      ; r11 = curr size
    
    xor rcx, rcx               ; rcx = counter

get_loop:

    cmp r11, rcx
    jbe return_null

    mov rsi, r9

    mov rax, [r10] ; curr bucket
    mov rbx, rcx

    shl rbx, 0x5
    add rax, rbx
    mov rdi, [rax]

    call mstrcmp

    cmp rax, 0x0
    je return_ptr
    inc rcx
    jmp get_loop

return_ptr:
    mov rax, [r10] ; curr bucket
    mov rbx, rcx
    
    shl rbx, 0x5
    add rax, rbx
    add rax, 0x8
    
    ret

return_null:
    xor rax, rax
    ret
