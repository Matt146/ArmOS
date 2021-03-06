[org 0x7c00]
[bits 16]

; Force cs to 0
jmp 0x0000:setup_segment_registers

; Define constants on where to load stuff into memory
BIOS_MEMORY_MAP_OFFSET  equ 0x7E00
KERNEL_OFFSET           equ 0x9000

; Define constants for the 1gb identity-mapped page table
P4	equ 0x15000	; Page-map level-4 table (PML4T)
P3	equ 0x16000	; Page-directory pointer table (PDPT)
P2	equ 0x17000	; Page-directory table (PDT)

DAP:
    db 0x10 ; DAP size (0x10) [1 byte]
    db 0x00 ; Unused (0x00) [1 byte]
    dw 0x0F ; Number of sectors to be read [2 bytes] [Some BIOS's can only go up to 127]
    dd KERNEL_OFFSET ; Segment:offset pointer into where the data is loaded in memory [4 bytes]
    dq 0x01 ; LBA start address of the first sector to be read [8 bytes]

; Force segment registers to 0, except for cs
setup_segment_registers:
    xor ax, ax
    mov ss, ax
    mov ds, ax
    mov es, ax

setup_stack:
    mov sp, 0x7C00
    mov bp, sp

set_video_mode:
    mov ah, 0x00
    mov al, 0x03
    int 0x10

; If this doesn't work, you need to jump out of the year 1990
enable_a20:
    in al, 0x92
    or al, 2
    out 0x92, al

load_kernel:
    mov ah, 0x42
    mov si, DAP
    int 0x13
    jc load_kernel_error
    xor ebx, ebx
    mov di, BIOS_MEMORY_MAP_OFFSET
    jmp read_bios_memory_map

load_kernel_error:
    mov ah, 0x0e
    mov al, 'F'
    int 0x10
    jmp $
    hlt

read_bios_memory_map_error:
    mov ah, 0x0e
    mov al, 'H'
    int 0x10
    jmp $
    hlt

read_bios_memory_map:
    mov eax, 0xe820
    mov ecx, 0x18   ; acpi 3.0 support :D
    mov edx, 0x534D4150
    int 0x15

    ; If carry is set, there's an error
    jc read_bios_memory_map_error

    ; If ebx = 0, then we've reached the end.
    add di, 0x18
    cmp ebx, 0x00
    jne read_bios_memory_map

switch_to_pmode:
    cli
    lgdt [gdt_descriptor]
    mov eax, cr0
    or eax, 0x1
    mov cr0, eax
    jmp CODE_SEG:init_pm

gdt_start:

gdt_null:
    dd 0x0
    dd 0x0

gdt_code:
    ; base = 0x0, limit=0xfffff
    ; 1st  flags: (present )1 (privilege )00 (descriptor  type)1 -> 1001b
    ; type  flags: (code)1 (conforming )0 (readable )1 (accessed )0 -> 1010b
    ; 2nd  flags: (granularity )1 (32-bit  default )1 (64-bit  seg)0 (AVL)0 -> 1100b
    dw 0xffff    ; Limit (bits 0-15)
    dw 0x0       ; Base (bits 0-15)
    db 0x0       ; Base (bits 16-23)
    db 10011010b ; 1st flags , type  flags
    db 11001111b ; 2nd flags , Limit (bits  16-19)
    db 0x0       ; Base (bits  24 -31)

gdt_data: ;the  data  segment  descriptor
    ; Same as code  segment  except  for  the  type  flags
    ; type  flags: (code)0 (expand  down)0 (writable )1 (accessed )0 -> 0010b
    dw 0xffff      ; Limit (bits  0-15)
    dw 0x0         ; Base (bits  0-15)
    db 0x0         ; Base (bits  16 -23)
    db 10010010b   ; 1st flags , type  flags
    db 11001111b   ; 2nd flags , Limit (bits  16-19)
    db 0x0         ; Base (bits  24 -31)

gdt_end:           ; The  reason  for  putting a label  at the  end of the
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

[bits 32]
init_pm:
    mov ax, DATA_SEG
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

    jmp prep_switch_to_long_mode

prep_switch_to_long_mode:
	; zero eax
	xor eax, eax
	mov ecx, 0x17000

zero_tables:
	mov [ecx], dword 0	; write zero
	dec ecx
	cmp ecx, 0x15000
	jg zero_tables
	jmp map_P4_P3_tables

map_P4_P3_tables:
	; Map first P4 entry to P3 table
	mov eax, P3 | 0b11 ; present + writable
	mov [P4], eax

	; Map first P3 entry to P2 table
	mov eax, P2 | 0b11 ; present + writable	
	mov [P3], eax

	mov ecx, 0

map_P2_table:
	; map ecx-th P2 entry to a huge page that starts at address 2 MiB*ecx
	mov eax, 0x200000
	mul ecx
	or eax, 0b10000011
	mov [P2 + ecx * 8], eax ; map ecx-th entry

	inc ecx
	cmp ecx, 512
	jne map_P2_table

load_page_tables:
	; load top-level page table to cr3, so the CPU can see it
	; CR3 is the register that contains the top-level page table
	; address
	mov eax, P4
	mov cr3, eax

switch_to_compatibility_mode:
	; enable PAE-flag in cr4 (physical address extension)
	; Allows for 36-bit protected mode addresses
	mov eax, cr4
	or eax, 1 << 5
	mov cr4, eax

	; set the long mode bit in the EFER MSR (model specific register)
	; This allows us to switch to long mode
	mov ecx, 0xC0000080	; the MSR we are reading/writing to
	rdmsr			; put the value of the MSR specified by ecx into edx:eax
	or eax, 1 << 8		; set the long mode bit
	wrmsr			; write it back to the MSR

	; enable paging in the cr0 register
	mov eax, cr0
	or eax, 1 << 31
	mov cr0, eax

    ; Jump over the gdt64 to the get into 64-bit mode subroutine
    jmp get_into_64_bit_mode

gdt64_start:

gdt64_null:
	dq 0

gdt64_code:
	dq (1<<43) | (1<<44) | (1<<47) | (1<<53)

gdt64_end:

gdt64_descriptor:
	dw $ - gdt64_start - 1	; 2 bytes for size
	dq gdt64_start		; 8 bytes for starting address

; gdt_64 constants
GDT64_CODE_SEG  equ  gdt64_code  - gdt64_start

get_into_64_bit_mode:
	; load the 64-bit GDT
	lgdt [gdt64_descriptor]

	; Far jump to set cs:rip
	jmp GDT64_CODE_SEG:LONG_MODE_START

[bits 64]
LONG_MODE_START:
	; YAY! We're in 64-bit submode now	
	; Time to load 0 into all data segment registers
	xor rax, rax
    xor rbx, rbx
    xor rcx, rcx
    xor rdx, rdx
    xor rsi, rsi
    xor rdi, rdi
    mov rbp, 0x90000
    mov rsp, rbp
    xor r8, r8
    xor r9, r9
    xor r10, r10
    xor r11, r11
    xor r12, r12
    xor r13, r13
    xor r14, r14
    xor r15, r15
	mov ss, ax
	mov ds, ax
	mov es, ax
	mov fs, ax
	mov gs, ax

    ; Disable the PIC, so we can use the APIC
    mov al, 0xff
    out 0xa1, al
    out 0x21, al

    ; jmp $
    sti

	; Jump to kernel
	call KERNEL_OFFSET

	jmp $	; HANG

jmp $

times 510-($-$$) db 0
dw 0xaa55