[bits 64]
global outb
outb:
    ; Function prologue
    push rbp
    mov rbp, rsp

    mov rdx, rdi
    mov rax, rsi
    out dx, al

    ; Function epilogue
    mov rsp, rbp
    pop rbp
    ret

global inb
inb:
    ; Function prologue
    push rbp
    mov rbp, rsp

    xor rax, rax
    mov rdx, rdi
    in al, dx
    xor rdx, rdx

    ; Function epilogue
    mov rsp, rbp
    pop rbp
    ret