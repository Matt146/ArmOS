[bits 64]
global refresh_page_tables
refresh_page_tables:
    ; Function prologue
    push rbp
    mov rbp, rsp

    push rax
    push rbx
    mov rbx, qword [0x15000]
    mov rax, cr3
    mov cr3, rax
    pop rbx
    pop rax

    ; Function epilogue
    mov rsp, rbp
    pop rbp
    ret