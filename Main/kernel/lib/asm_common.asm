[bits 64]
global isr_common_stub
isr_common_stub:
    hlt
    iretq   ; fuck off

extern ps2_irq1_handler
global ps2_irq1_handler_stub
ps2_irq1_handler_stub:
    call ps2_irq1_handler
    iretq

extern ps2_irq12_handler
ps2_irq12_handler_stub:
    iretq

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
    push rdx
    cli

mutex_spin:
    mov rax, 0
    mov rdx, 1
    lock cmpxchg [rdi], rdx
    jnz mutex_spin

mutex_lock_ret:
    pop rdx
    mov rsp, rbp
    pop rbp
    ret

global mutex_unlock
mutex_unlock:
    push rbp
    mov rbp, rsp
    mov [rdi], byte 0

mutex_unlock_ret:
    sti
    mov rsp, rbp
    pop rbp
    ret