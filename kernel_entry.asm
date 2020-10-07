[org 0x8c00]
k_entry:
    mov ebx, 0xb8000
    mov [ebx], word (0x1F << 8) | 'h'
    ; mov [ebx], word 'A '

    jmp $ ; hang

times 0x8000-($-$$) db 0
