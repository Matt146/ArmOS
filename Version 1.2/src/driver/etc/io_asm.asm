[bits 64]
global out_b
out_b:
    push rbp
    mov rbp, rsp

    mov rdx, rdi
    mov rax, rsi
    out dx, al      ; Output byte AL to I/O port address in DX

    mov rsp, rbp
    pop rbp
    ret

global in_b
in_b:
    push rbp
    mov rbp, rsp

    mov rdx, rdi
    in al, dx      ; Input byte from I/O port in DX into AL

    mov rsp, rbp
    pop rbp
    ret

global DEBUG
DEBUG:
    push rbp
    mov rbp, rsp

    mov rax, 0xDEADBEEF

    mov rsp, rbp
    pop rbp
    ret