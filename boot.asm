;**************************************
; bootloader.asm
;	- A really simple bootloader
;
; Date: 07/16/20
; By: Matt
;**************************************

org 0x7c00 ; We are loaded by BIOS at 0x7c00
bits 16	   ; We are in 16-bit real mode

; First, we need to setup the stack
mov	bp, 0x8000
mov	sp, bp

boot:
	mov	ah, 0x0e
	mov	al, '!'
	mov	bh, 0x0
	mov	bl, 0x07

	int	0x10

; Setup the GDT now
gdt_start:

gdt_null:	; Here, we setup the null descriptor
	dd	0x0
	dd	0x0

gdt_code:	; Here, we setup the code segment descriptor
	dw	0xffff		; Limit (bits 0-15)
	dw	0x0		; Base (0-15)
	dw	0x0		; Base (16-23)
	db	10011010b	; 1st flags, type flags
	db	11001111b	; 2nd flags, limit (bits 16-19)
	db	0x0		; Base (bits 24-31)

gdt_data:	; Here, we setup the data segment descriptor
	; Same as code segment except for type flags
	; type flags: (code)0 (expand down)0 (writable)1 (accessed)0 --> 0010b
	dw	0xffff		; Limit (bits 0-15)
	dw	0x0		; Base (bits 0-15)
	db	0x0		; Base (bits 16-23)
	db	10010010b	; 1st flags, type flags
	db	0x0		; Base (bits 24-31)

gdt_end:	; The reason for putting a label at the end of GDT is so we can
		; have the assembler calculate the size of the GDT for the GDT descriptor
		; below

gdt_descriptor:
	dw	gdt_end - gdt_start - 1	; Size of our GDT, always less one of the true size
	dd	gdt_start		; Start address of our GDT

CODE_SEG equ gdt_code - gdt_start
DATA_SEG equ gdt_data - gdt_start

; Here, we are going to switch to protected mode
switch_to_protected_mode:
	cli				; Disable interrupts
	lgdt	[gdt_descriptor]	; Load the GDT that took us forever to prepare...

	; Now, we can actually switch over to protected mode.
	; We do that by setting the first bit of a special CPU register
	; known as cr0
	mov	eax, cr0
	or	eax, 0x1
	mov	cr0, eax

	jmp CODE_SEG:start_protected_mode ; Far jump to flush the current CPU state
						; and ensure that you are in protected mode

bits 32
start_protected_mode:
	; WOO HOO! WE'RE IN PROTECTED MODE NOW!
	mov ax, DATA_SEG	; Now in PM, our old segments are meaningless
	mov ds, ax		; so we point our segment registers to the data selector we defined
	mov ss, ax		; in our GDT
	mov es, ax
	mov fs, ax
	mov gs, ax

	mov ebp, 0x90000	; Update our stack position so it is right at the top of the free space
	mov esp, ebp

	call BEGIN_PM		; Now, in our next jump, we'll be code in protected mode

BEGIN_PM:
	; YAY! START DOING PROTECTED MODE STUFF NOW!
	jmp $

times 510 - ($-$$) db 0 ; We have 512 bytes in the MBR. Write 0 to the rest

dw 0xaa55 ; Write the boot signature
