[org 0x8c00]
[bits 64]

jmp $

times 0x8000-($-$$) db 0
