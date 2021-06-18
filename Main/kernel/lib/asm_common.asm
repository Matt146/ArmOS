[bits 64]
global isr_common_stub
isr_common_stub:
    hlt
    iretq   ; fuck off

global __lgdt
__lgdt:
    lgdt [rdi]
    ret

global gdt_reset_segment_registers
gdt_reset_segment_registers:
    push rbp
    mov rbp, rsp

    cli
    mov rax, seggs
    push di                 ; di contains the cs selector value we want
    push rax
    retfq
seggs:
    mov ax, si              ; si contains the data segment selector value we want
    mov ds, si
    mov es, si
    mov fs, si
    mov gs, si
    mov ss, si
    sti

    mov rsp, rbp
    pop rbp
    ret

global mutex_lock
mutex_lock:
    push rbp
    mov rbp, rsp

mutex_spin:
    cmp [rdi], byte 0
    jz mutex_spin
    jnz mutex_lock_ret

mutex_lock_ret:
    lock inc byte [rdi] 

    mov rsp, rbp
    pop rbp
    ret

global mutex_unlock
mutex_unlock:
    push rbp
    mov rbp, rsp

    mov [rdi], byte 0

mutex_unlock_ret:
    mov rsp, rbp
    pop rbp
    ret