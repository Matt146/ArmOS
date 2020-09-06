[org 0x7c00]
[bits 16]
; Global vars
KERNEL_OFFSET equ 0x1000

mov bp, 0x9000  ; Setup the stack
mov sp, bp
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
    jmp $
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
    dw  gdt_end  - gdt_start  - 1
    dd  gdt_start

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

    ; Far jump
    jmp CODE_SEG:init_pm

[bits 32]
[extern kmain]
init_pm:
    mov ax, DATA_SEG
    mov ds, ax
    mov ss, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    mov ebp, 0x90000
    mov esp, ebp

    call BEGIN_PM

print_string_pm:
    mov ebx, 0xb8000

print_string_pm_loop_start:
    cmp ebx, 0xB8FA0
    jg print_string_pm_loop_end
    cmp ebx, 0xB87D0
    je print_string_pm_h
    jmp print_string_pm_empty

print_string_pm_empty:
    mov [ebx], word (0x1F << 8) | '.'
    jmp print_string_pm_loop_inc

print_string_pm_h:
    mov [ebx], word (0x1F << 8) | 'h'
    jmp print_string_pm_loop_inc

print_string_pm_loop_inc:
    times 2 inc ebx
    jmp print_string_pm_loop_start

print_string_pm_loop_end:
    ret

BEGIN_PM:
    ; call print_string_pm    ; Use our 32-bit print routine
    call KERNEL_OFFSET
    jmp $                   ; Hang

times 510-($-$$) db 0

dw 0xaa55

times 0x8000-($-$$) db 0