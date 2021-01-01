[bits 64]
global refresh_page_tables
refresh_page_tables:
    ; Function prologue
    push rbp
    mov rbp, rsp

    push rax
    mov rax, cr3
    mov cr3, rax
    pop rax

    ; Function epilogue
    mov rsp, rbp
    pop rbp
    ret