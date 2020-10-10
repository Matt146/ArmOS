; NASM directives:
; org  - tells where the program is supposed to be loaded in memory
;	 in this case, 0x7c00 is where the BIOS leaves us
; bits - encodes instructions for 16 bits
[org 0x7c00]
[bits 16]

; Set up segment registers
mov ax, 0x0000
mov ds, ax
mov es, ax
mov ss, ax

; Global vars
KERNEL_OFFSET equ 0x8c00

; Set up the stack
mov sp, 0xAAAA

; Load the kernel using BIOS functions
call load_kernel

; Now, we go to protected mode
jmp switch_to_protected_mode

load_kernel:
    call disk_load
    ret

disk_load:
    mov ah, 0x42
    mov si, DAP
    int 0x13
    jc disk_error
    ret

DAP:
    db 0x10
    db 0x0
    dw 15
    dd KERNEL_OFFSET
    dq 1

disk_error:
    mov ah, 0x0e
    mov al,'F'
    int 0x10
    jmp $

; GDT
gdt_start:

gdt_null:
    dd 0x0
    dd 0x0

gdt_code:
    ; base = 0x0, limit=0xfffff
    ; 1st  flags: (present )1 (privilege )00 (descriptor  type)1 -> 1001b
    ; type  flags: (code)1 (conforming )0 (readable )1 (accessed )0 -> 1010b
    ; 2nd  flags: (granularity )1 (32-bit  default )1 (64-bit  seg)0 (AVL)0 -> 1100b
    dw 0xffff   ; Limit (bits 0-15)
    dw 0x0      ; Base (bits 0-15)
    db 0x0      ; Base (bits 16-23)
    db  10011010b ; 1st flags , type  flags
    db  11001111b ; 2nd flags , Limit (bits  16-19)
    db 0x0         ; Base (bits  24 -31)

gdt_data: ;the  data  segment  descriptor
    ; Same as code  segment  except  for  the  type  flags
    ; type  flags: (code)0 (expand  down)0 (writable )1 (accessed )0 -> 0010b
    dw 0xffff     ; Limit (bits  0-15)
    dw 0x0         ; Base (bits  0-15)
    db 0x0         ; Base (bits  16 -23)
    db  10010010b ; 1st flags , type  flags
    db  11001111b ; 2nd flags , Limit (bits  16-19)
    db 0x0         ; Base (bits  24 -31)

gdt_end:         ; The  reason  for  putting a label  at the  end of the
                 ; GDT is so we can  have  the  assembler  calculate
                 ; the  size of the  GDT  for  the GDT  decriptor (below)

; GDT descriptor
gdt_descriptor:
    dw  gdt_end  - gdt_start  - 1	; 16 bits for size of GDT
    dd  gdt_start			; 32 bits for where the GDT starts

; Define  some  handy  constants  for  the  GDT  segment  descriptor  offsets , which
; are  what  segment  registers  must  contain  when in  protected  mode.  For  example ,
; when we set DS = 0x10 in PM , the  CPU  knows  that we mean it to use  the
; segment  described  at  offset 0x10 (i.e. 16  bytes) in our GDT , which in our
; case is the  DATA  segment  (0x0 -> NULL; 0x08  -> CODE; 0x10  -> DATA)
CODE_SEG  equ  gdt_code  - gdt_start
DATA_SEG  equ  gdt_data  - gdt_start

switch_to_protected_mode:
    cli     ; Disable interrutps rn

    ; Tell the CPU about the GDT we just prepared
    lgdt [gdt_descriptor]

    ; Set cr0's first bit
    mov eax, cr0
    or eax, 0x1
    mov cr0, eax

    ; Far jump to set CS
    jmp CODE_SEG:init_pm

[bits 32]
init_pm:
    ; For flat memory model
    mov ax, DATA_SEG	; put the offset of the data segment descriptor in the GDT into ax
    mov ds, ax		; now put ax into ds - can't set ds directly
    mov ss, ax		; set stack segment to ax
    mov es, ax		; set extra segment to ax
    mov fs, ax		; set fs to ax
    mov gs, ax		; set gs to ax

    ; now set the base pointer and the stack pointer
    ; to set up the stack
    mov ebp, 0x90000
    mov esp, ebp

    sti ; re-enable interrupts

    ; as of now, we have completely set up protected mode
    ; jump to prepare our switch to long mode
    jmp prep_switch_to_long_mode


; Page tables for hierarchical paging
PML4T	equ 0x10000	; Page-map level-4 table
PDPT	equ 0x11000	; Page-directory pointer table
PDT	equ 0x12000	; Page-directory table
PT	equ 0x13000	; Page table

prep_switch_to_long_mode:
	; Enable paging
	mov eax, cr0                                   ; Set the A-register to control register 0.
	and eax, 01111111111111111111111111111111b     ; Clear the PG-bit, which is bit 31.
	mov cr0, eax

	; Clear page tables
	mov edi, PML4T	; load PML4T in edi
	mov cr3, edi	; set cr3 to point to PML4T
	xor eax, eax	; clear eax
	mov ecx, 4096	; set ecx to 4096
	rep stosd	; clear memory
	mov edi, cr3	; set di to cr3

	; Make page tables point to each other
	mov dword[edi], 0x11003
	add edi, 0x1000
	mov dword [edi], 0x12003
	add edi, 0x1000
	mov dword [edi], 0x13003
	add edi, 0x1000

	; Identity map the first two mb
	mov ebx, 0x00000003
	mov ecx, 512

.set_entry:
	mov dword [edi], ebx
	add ebx, 0x1000
	add edi, 8
	loop .set_entry

switch_to_long_mode:
	; set the LM bit
	mov ecx, 0xC0000080
	rdmsr
	or eax, 1 << 8
	wrmsr

	; enable paging
	mov eax, cr0
	or eax, 1 << 31
	mov cr0, eax

	; Now in compatibility mode. We need to get into 64-bit mode
	jmp switch_to_64_bit_submode


GDT64:                           ; Global Descriptor Table (64-bit).
	.Null: equ $ - GDT64         ; The null descriptor.
	dw 0xFFFF                    ; Limit (low).
	dw 0                         ; Base (low).
	db 0                         ; Base (middle)
	db 0                         ; Access.
	db 1                         ; Granularity.
	db 0                         ; Base (high).
	.Code: equ $ - GDT64         ; The code descriptor.
	dw 0                         ; Limit (low).
	dw 0                         ; Base (low).
	db 0                         ; Base (middle)
	db 10011010b                 ; Access (exec/read).
	db 10101111b                 ; Granularity, 64 bits flag, limit19:16.
	db 0                         ; Base (high).
	.Data: equ $ - GDT64         ; The data descriptor.
	dw 0                         ; Limit (low).
	dw 0                         ; Base (low).
	db 0                         ; Base (middle)
	db 10010010b                 ; Access (read/write).
	db 00000000b                 ; Granularity.
	db 0                         ; Base (high).
	.Pointer:                    ; The GDT-pointer.
	dw $ - GDT64 - 1             ; Limit.
	dq GDT64                     ; Base.

switch_to_64_bit_submode:
	lgdt [GDT64.Pointer]
	jmp GDT64.Code:Realm64

[bits 64]
Realm64:
	cli                           ; Clear the interrupt flag.
	mov ax, GDT64.Data            ; Set the A-register to the data descriptor.
	mov ds, ax                    ; Set the data segment to the A-register.
	mov es, ax                    ; Set the extra segment to the A-register.
	mov fs, ax                    ; Set the F-segment to the A-register.
	mov gs, ax                    ; Set the G-segment to the A-register.
	mov ss, ax                    ; Set the stack segment to the A-register.
	mov edi, 0xB8000              ; Set the destination index to 0xB8000.
	mov rax, 0x1F201F201F201F20   ; Set the A-register to 0x1F201F201F201F20.
	mov ecx, 500                  ; Set the C-register to 500.
	rep stosq                     ; Clear the screen.
	hlt                           ; Halt the processor.

times 510-($-$$) db 0
dw 0xaa55
