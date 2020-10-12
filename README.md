# MattOS

This is an OS that I'm writing myself totally from scratch.

## Current progress

- Kernel loaded using BIOS interrupt int 0x13, ah=0x42
- Implemented enabling of a20 line in 3 ways:
        - Keyboard controller method
        - BIOS method
        - Fast a20
- Booted into protected mode
- Booted into long mode from protected mode
- Booted into 64-bit submode of long mode
- Next up: setting up IDT and interrupts
