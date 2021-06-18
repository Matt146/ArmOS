[bits 16]
[org 0x70000]

jmp 0x0000:coregobrr

data_passthrough:
    times 0x3000 db 0

sched_ap_GDTR:
    dw 39
    dq (0x70000 + 0x18)

sched_ap_IDTR:
    dw 4095
    dq (0x70000 + 0x18 + 0x1000)

coregobrr:
    cli
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov ss, ax
    mov gs, ax

    in al, 0x92
    or al, 2
    out 0x92, al

    lgdt [sched_ap_GDTR]
    mov eax, cr0        ; Set pmode enable enable bit
    or eax, 0x1
    mov cr0, eax
    jmp 0x8:reload_cs

[bits 32]
reload_cs:
    mov ax, 0x10 ; reload data seg registers
    mov ss, ax
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    mov eax, dword [0x70000 + 0x10] ; load cr3 to enable paging
    mov cr3, eax

[bits 64]
    ; set the long mode bit in the EFER MSR (model specific register)
	; This allows us to switch to long mode
	mov ecx, 0xC0000080
	rdmsr
	or eax, 1 << 8
	wrmsr

    ; enable paging
    mov rax, cr0        ; Set paging enable bit
    or rax, 0x80000000
    mov cr0, rax

    lidt [sched_ap_IDTR]

    cli
    hlt