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
    ; Save processor state of GPR's
    ; Notice how we don't push rsp - that's bc
    ; that's pushed on automatically when an interrupt occurs.
    ; When an interrupt occurs, here's what's pushed onto stack
    ; before the CPU jumps to the correct ISR:
    ; 1. SS
    ; 2. RSP
    ; 3. RFLAGS
    ; 4. CS
    ; 5. RIP
    ; 6. Error code (optional)
    push rax
    push rbx
    push rcx
    push rdx
    push rbp
    push rsi
    push rdi
    push r8
    push r9
    push r10
    push r11
    push r12
    push r13
    push r14
    push r15

    ; Save processor state of segment registers

    ; Reload data segments to kernel data segements.
    ; When an interrupt occurs, cs is already loaded
    ; as the kernel code segment descriptor,
    ; as specified in the IDT.
    mov rax, 0
    mov ds, rax
    mov fs, rax
    mov es, rax
    mov gs, rax

    ; Do work here - INFINITE LOOP FOR DEBUG
    mov rax, 0xdeadbeaf
    jmp $
    hlt

    ; Returning from interrupt
    ; iretq pops off stuff pushed in before the ISR
    ; is called (Error code, rip, cs, etc.), but must be called
    ; last since these were pushed on first onto the stack
    pop r15
    pop r14
    pop r13
    pop r12
    pop r11
    pop r10
    pop r9
    pop r8
    pop rdi
    pop rsi
    pop rbp
    pop rdx
    pop rcx
    pop rbx
    pop rax
    iretq