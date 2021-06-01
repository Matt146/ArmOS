global flush_cr3
flush_cr3:
    push rbp
    mov rbp, rsp

    push rax
    mov rax, cr3
    mov cr3, rax
    pop rax

    mov rsp, rbp
    pop rbp
    ret

global set_cr3
set_cr3:
    push rbp
    mov rbp, rsp

    cli
    mov rax, rdi
    mov cr3, rax
    sti

    mov rsp, rbp
    pop rbp
    ret

global get_rsp:
get_rsp:
    push rbp
    mov rbp, rsp

    mov rax, rsp

    mov rsp, rbp
    pop rbp
    ret