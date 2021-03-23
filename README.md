# MattOS

This is an OS that I'm writing myself totally from scratch.

## Current progress

- Loaded kernel (int 0x13, ah=0x42)
- A20 - Fast A20
- Protected Mode
- Long Mode (64-bit mode)
- Interrupts/IDT
- PMM
- Serial COM1

## Future Features

- VMM
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
