[bits 64]
global load_idt
load_idt:
    push rbp
    mov rbp, rsp

    lidt [rdi]

    mov rsp, rbp
    pop rbp
    ret

global isr_common_stub
isr_common_stub:
    ; When an interrupt occurs, here's what's pushed onto stack
    ; before the CPU jumps to the correct ISR:
    ; 1. SS
    ; 2. RSP
    ; 3. RFLAGS
    ; 4. CS
    ; 5. RIP
    ; 6. Error code (optional)
    ; Do work here - INFINITE LOOP FOR DEBUG
    mov rax, 0xdeadbeef
    iretq

global APIC_TIMER_STUB
APIC_TIMER_STUB:
    mov rax, 0x12345678
    jmp $
    hlt
    iretq

global VIEW_RFLAGS
VIEW_RFLAGS:
    push rbp
    mov rbp, rsp

    pushfq
    pop rax

    mov rsp, rbp
    pop rbp
    ret