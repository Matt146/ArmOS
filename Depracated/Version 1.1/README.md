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
- Setup basic exception handlers
- Setup kernel memory mapping

## Future Features

- Read BIOS memory map
- Continue work on VMM
- Work on PMM
- ACPI
- APIC
- PCI Driver
- Scheduler
- Disk driver

## Build

### Download:
 - Download source using `git clone`
### Requirements:
- NASM
- GCC
- GNU Linker
- QEMU
- GNU Make
- dd
### Run:
- Run with `make`

### Caution:
I am not responsible for any damage to the hardware that you choose to run my code on. By running this code, you are consciously aware of the associated risks.
