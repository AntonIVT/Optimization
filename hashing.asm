    default rel
    global hashing_function
 
section .text

; rdi = string
hashing_function:
    xor rax, rax

hashing_loop:

    crc32 rax, QWORD [rdi]

    add rdi, 8
    cmp BYTE [rdi], 0
    jne hashing_loop
    
    ret

