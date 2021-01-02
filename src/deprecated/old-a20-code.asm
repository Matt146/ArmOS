; ===========================================
;	FUNCTIONS TO ENABLE A20 GATE
; ===========================================
enable_a20_main:
    push bp
    mov bp, sp

    ; Try the keyboard controller method
    call enable_a20_keyboard
    call check_a20
    cmp ax, 1
    je enable_a20_main_end

    ; Try the bios method
    call enable_a20_bios
    call check_a20
    cmp ax, 1
    je enable_a20_main_end

    ; Try the fast a20 method
    call enable_a20_fast
    call check_a20
    cmp ax, 1
    je enable_a20_main_end

    ; If none of these, work just halt and print "B"
    jmp enable_a20_main_fail

enable_a20_main_end:
    mov sp, bp
    pop bp
    ret

enable_a20_main_fail:
   mov ah, 0x0e 
   mov al, 'B'
   int 0x10
   cli
   hlt
   jmp $

; The keybaord control method to enable 
; the a20 gate
enable_a20_keyboard:
    push bp
    mov bp, sp

    mov al, 0xdd
    out 0x64, al

    mov sp, bp
    pop bp
    ret

; The BIOS method to enable the a20 gate
enable_a20_bios:
    push bp
    mov bp, sp

    mov ax, 0x2401
    int 0x15

    mov sp, bp
    pop bp
    ret

; Fast a20 method - fast, but dangerous ;)
enable_a20_fast:
    push bp
    mov bp, sp
    
    mov al, 2
    out 0x92, al

    mov sp, bp
    pop bp
    ret


; Ez way of checking a20. Really helpful
check_a20:
    pushf
    push ds
    push es
    push di
    push si
 
    cli
 
    xor ax, ax ; ax = 0
    mov es, ax
 
    not ax ; ax = 0xFFFF
    mov ds, ax
 
    mov di, 0x0500
    mov si, 0x0510
 
    mov al, byte [es:di]
    push ax
 
    mov al, byte [ds:si]
    push ax
 
    mov byte [es:di], 0x00
    mov byte [ds:si], 0xFF
 
    cmp byte [es:di], 0xFF
 
    pop ax
    mov byte [ds:si], al
 
    pop ax
    mov byte [es:di], al
 
    xor ax, ax
    je check_a20__exit
 
    mov ax, 1
 
check_a20__exit:
    pop si
    pop di
    pop es
    pop ds
    popf
 
    ret