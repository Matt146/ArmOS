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