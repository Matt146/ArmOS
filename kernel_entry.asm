[org 0x8C00]
k_entry:
    mov ebx, 0xb8000
    mov [ebx], word (0x1F << 8) | 'A'
    ; mov [ebx], 'A '

    jmp $ ; hang

times 0x8000-($-$$) db 0