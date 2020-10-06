[org 0x8c00]
k_entry:
    mov ebx, 0xb8002
    mov [ebx], word (0x1F << 8) | 'h'
    ; mov [ebx], word 'A '

    ; call print_string_pm

    jmp $ ; hang

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

times 0x8000-($-$$) db 0
