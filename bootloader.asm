;**************************************
; bootloader.asm
;	- A really simple bootloader
;
; Date: 07/16/20
; By: Matt
;**************************************

org 0x7c00 ; We are loaded by BIOS at 0x7c00
bits 16	; We are in 16-bit real mode

start: jmp loader

;**************************************
;	OEM Parameter Block
;**************************************
TIMES 0Bh-$+start DB 0

bpbBytesPerSector:  	DW 512
bpbSectorsPerCluster: 	DB 1
bpbReservedSectors: 	DW 1
bpbNumberOfFATs: 	    DB 2
bpbRootEntries: 	    DW 224
bpbTotalSectors: 	    DW 2880
bpbMedia: 	            DB 0xF0
bpbSectorsPerFAT: 	    DW 9
bpbSectorsPerTrack: 	DW 18
bpbHeadsPerCylinder: 	DW 2
bpbHiddenSectors: 	    DD 0
bpbTotalSectorsBig:     DD 0
bsDriveNumber: 	        DB 0
bsUnused: 	            DB 0
bsExtBootSignature: 	DB 0x29
bsSerialNumber:	        DD 0xa0a1a2a3
bsVolumeLabel: 	        DB "MOS FLOPPY "
bsFileSystem: 	        DB "FAT12   "

;**************************************
; Bootloader Entry Point
;**************************************
msg	db "Welcome to MattOS", 0

print_msg:
	lodsb
	or	al, al
	jz	print_msg_done
	mov	ah, 0xe
	int	0x10
	jmp	print_msg

print_msg_done:
	ret

reset_disk:
	mov	ah, 0		; reset floppy disk function
	mov	dl, 0		; dl is the drive number
	int	0x13		; trigger the interrupt
	jc	reset		; triggering the interrupt will return two things:
				; status code, which is stored in ah
				; if it failed, which is represented by the carry flag.
				; if the carry flag is clear, it was a success.
				; if the carry flag was set, there was an error

read_disk:
	mov	ah, 0x02	; read from disk function
	mov	al, 1		; specify that we want to read only 1 sector
	mov	ch, 1		; say we are reading from track 1
	mov	cl, 2		; we want to read the second sector
	mov	dh, 0		; the first head
	mov	dl, 0		; the drive number: 0
	int	0x13		; trigger the interrupt
	jc	read_disk	; try again if error

	jmp	0x1000:0x0	; jump to execute the sector

loader:
	; Print character example:
	; xor	bx, bx		; bx has to be set to 0
	; mov	ah, 0x0e	; enter tty mode by setting ah to 0x0e
	; mov	al, 'A'		; mov the letter 'A' into al
	; int	0x10		; perform the interrupt to display the text onto the screen

	mov	si, msg		; our message to print
	call	print_msg	; call our print function

	cli			; disable interrupts
	hlt			; halt the system
	

times 510 - ($-$$) db 0 ; We have 512 bytes in the MBR. Write 0 to the rest

dw 0xaa55 ; Write the boot signature
