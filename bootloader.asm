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
msg db "Welcome to MattOS", 0

loader:
	xor bx, bx	; bx has to be set to 0
	mov ah, 0x0e	; enter tty mode by setting ah to 0x0e
	mov al, 'A'	; mov the letter 'A' into al
	int 0x10	; perform the interrupt to display the text onto the screen

	cli	; disable interrupts
	hlt	; halt the system
	

times 510 - ($-$$) db 0 ; We have 512 bytes in the MBR. Write 0 to the rest

dw 0xaa55 ; Write the boot signature
