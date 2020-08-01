;**************************************
; bootloader.asm
;	- A really simple bootloader
;
; Date: 07/16/20
; By: Matt
;**************************************

org 0x7c00 ; We are loaded by BIOS at 0x7c00
bits 16	   ; We are in 16-bit real mode

;**********************************************
; Boot setup functions
;**********************************************
; Setup the stack here
mov	bp, 0x7FFF
mov	sp, bp

; Define pages here
pml4 equ 0x1000
pml3 equ 0x2000
pml2 equ 0x3000
hh_pml2 equ 0x4000
hh_pml3 equ 0x5000

; Boot from here
boot:
	; Do a silly interrupt
	mov	ah, 0x0e
	mov	al, '!'
	mov	bh, 0x0
	mov	bl, 0x07
	int	0x10
	
	; Check for long mode
	call	check_long_mode
	
	; If we can enter long mode, print
	; 'Y' and call enter_long_mode
	mov	ah, 0x0e
	mov	al, 'Y'
	mov	bh, 0x0
	mov	bl, 0x07
	int	0x10
	call	enter_long_mode
	
	; Now, halt the system because I feel like it
	cli
	hlt
	jmp	$

; Bootloader error handler
error:
	mov dword [0xb8000], 0x4f524f45
	mov dword [0xb8004], 0x4f3a4f52
	mov dword [0xb8008], 0x4f204f20
	mov byte  [0xb800a], al
	hlt

; Check if we can use long mode
check_cpuid:
	; Check if CPUID is supported by attempting to flip the ID bit (bit 21)
	; in the FLAGS register. If we can flip it, CPUID is available.

	; Copy FLAGS in to EAX via stack
	pushfd
	pop eax

	; Copy to ECX as well for comparing later on
	mov ecx, eax

	; Flip the ID bit
	xor eax, 1 << 21

	; Copy EAX to FLAGS via the stack
	push eax
	popfd

	; Copy FLAGS back to EAX (with the flipped bit if CPUID is supported)
	pushfd
	pop eax

	; Restore FLAGS from the old version stored in ECX (i.e. flipping the
	; ID bit back if it was ever flipped).
	push ecx
	popfd

	; Compare EAX and ECX. If they are equal then that means the bit
	; wasn't flipped, and CPUID isn't supported.
	cmp eax, ecx
	je .no_cpuid
	ret
.no_cpuid:
	mov al, "1"
	jmp error
check_long_mode:
	; test if extended processor info in available
	mov eax, 0x80000000    ; implicit argument for cpuid
	cpuid                  ; get highest supported argument
	cmp eax, 0x80000001    ; it needs to be at least 0x80000001
	jb .no_long_mode       ; if it's less, the CPU is too old for long mode

	; use extended info to test if long mode is available
	mov eax, 0x80000001    ; argument for extended processor info
	cpuid                  ; returns various feature bits in ecx and edx
	test edx, 1 << 29      ; test if the LM-bit is set in the D-register
	jz .no_long_mode       ; If it's not set, there is no long mode
	ret
.no_long_mode:
	mov al, "2"
	jmp error


;************************************************
; 64-bit: Enter into long mode
;************************************************
; How to setup paging
;	1.  Get the address of the P4 table from the CR3 register
;	2.  Use bits 39-47 (9 bits) as an index into P4 (2^9 = 512 = number of entries)
;	3.  Use the following 9 bits as an index into P3
;	4.  Use the following 9 bits as an index into P2
;	5.  Use the following 9 bits as an index into P1
;	6.  Use the last 12 bits as page offset (2^12 = 4096 = page size)
enter_long_mode:
	push bp
	mov bp, sp

	call setup_page_tables

	mov sp, bp
	pop bp
	ret

setup_page_tables:
	; Now load the page tables and enable paging
	; and add a gdt
	call enable_paging
	lgdt [gdt64.pointer]

	; Now, in order to truly enter long mode, we need
	; to load cs by far jumping
	; Now, print 'A' as a debugging thing
	; before you officially switch to long mode
	mov	ah, 0x0e
	mov	al, 'A'
	mov	bh, 0x0
	mov	bl, 0x07
	int	0x10
	jmp gdt64.code:long_mode_start

	mov sp, bp
	pop bp
	ret

enable_paging:
	push bp
	mov bp, sp
	
	; load P4 into the cr3 register (cpu uses this to access the P4 table)
	mov ax, pml4
	mov cr3, eax

	; enable PAE-flag in cr4 (Physical Address Extension)
	mov eax, cr4
	or eax, 1 << 5
	mov cr4, eax

	; set the long mode bit in the EFR MSR (model specific register)
	mov ecx, 0xC0000080
	rdmsr
	or eax, 1 << 8
	wrmsr

	mov sp, bp
	pop bp
	ret

gdt64:
	dq 0 ; zero entry
	.code:
		dq (1<<43) | (1<<44) | (1<<47) | (1<<53) ; code segment
.pointer:
	dw $ - gdt64 - 1
	dq gdt64

global long_mode_start
section .text
bits 64
long_mode_start:
	; print 'OKAY' to screen
	mov rax, 0x2f592f412f4b2f4f
	mov qword [0xb8000], rax
	cli
	hlt

;***********************************************
; Write the rest of the bytes and the boot
; signature
;***********************************************
times 510 - ($-$$) db 0 ; We have 512 bytes in the MBR. Write 0 to the rest

dw 0xaa55 ; Write the boot signature
