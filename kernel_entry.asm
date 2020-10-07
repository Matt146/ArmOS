[org 0x8c00]
bits 32

mov ebx, 0xb8000
mov [ebx], word (0x1F << 8) | 'A'

jmp $

times 0x8000-($-$$) db 0
