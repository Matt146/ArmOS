[bits 64]
global isr_common_stub
isr_common_stub:
    hlt
    iretq   ; fuck off

global gdt_reset_segment_registers
gdt_reset_segment_registers:
    lea rax, [rel .seggs]
    push word di
    push qword rax
    retf
.seggs:
    xor rax, rax
    mov ax, si
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax

    ret