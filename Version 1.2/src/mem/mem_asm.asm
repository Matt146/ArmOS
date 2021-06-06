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

global get_rsp:
get_rsp:
    push rbp
    mov rbp, rsp

    mov rax, rsp

    mov rsp, rbp
    pop rbp
    ret