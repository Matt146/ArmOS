[bits 16]
[org 0x70000]

jmp 0x7000:coregobrr

times 0x1000 - ($ - $$) db 0    ; Per core area at 0x70000
times 512 db 0                  ; 512 bytes for whatever I want

gdt_start:

gdt_null:
    dq 0x0

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

gdt_code2:
    ; base = 0x0, limit=0xfffff
    ; 1st  flags: (present )1 (privilege )00 (descriptor  type)1 -> 1001b
    ; type  flags: (code)1 (conforming )0 (readable )1 (accessed )0 -> 1010b
    ; 2nd  flags: (granularity )1 (32-bit  default )1 (64-bit  seg)0 (AVL)0 -> 1100b
    dw 0xffff    ; Limit (bits 0-15)
    dw 0x0       ; Base (bits 0-15)
    db 0b111       ; Base (bits 16-23)
    db 10011010b ; 1st flags , type  flags
    db 11001111b ; 2nd flags , Limit (bits  16-19)
    db 0x0       ; Base (bits  24 -31)

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
CODE_SEG2 equ  gdt_code2 - gdt_start

coregobrr:
    mov ax, 0x7000
    mov ds, ax
    mov es, ax
    mov ss, ax

    cli
    lgdt [gdt_descriptor]
    mov eax, cr0
    or eax, 0x1
    mov cr0, eax
    jmp CODE_SEG2:init_pmode        ; This uses CODE_SEG2 which has CS.BASE = 0x70000, since init_pmode will be assembled to only a relative address

[bits 32]
init_pmode:
    mov ax, DATA_SEG
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov fs, ax
    mov gs, ax
    jmp CODE_SEG:init_long_mode    ; Second far jump to load CS with the correct CS.BASE = 0x0 to avoid cringe

init_long_mode:
    sti
    ; Put the cr3 value passed from the per_core area in
    mov eax, [0x71000]
    mov cr3, eax

    ; Set 5th bit of CR4 - This is the PAE bit
    mov eax, cr4
	or eax, 1 << 5
	mov cr4, eax

    ; Set the 8th bit of the EFER MSR (LME bit)
    mov ecx, 0xC0000080	; the MSR we are reading/writing to
	rdmsr			; put the value of the MSR specified by ecx into edx:eax
	or eax, 1 << 8		; set the long mode bit
	wrmsr			; write it back to the MSR

    ; Enable paging in the cr0 register
	mov eax, cr0
	or eax, 1 << 31
    ;or eax, 1 << 16 No WP yet bro
	mov cr0, eax

    ; Load the GDT
    lgdt [0x71000+8]
    jmp 0x8:long_mode_reload_seg_regs

[bits 64]
long_mode_reload_seg_regs:
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov fs, ax
    mov gs, ax

    ; Load the IDT
    lidt [0x71000+18]

    ; Load the stack
    mov rsp, [0x71000+28]
    mov rbp, rsp

    hlt