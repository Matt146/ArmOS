# MattOS

This is an OS that I'm writing myself totally from scratch.

## Current progress

- Entered Long Mode
- Interrupts/IDT
- Serial COM1
- PMM
- VMM
- GDT

## WIP:

- VMM: Unmapping pages & checking if page is mapped
- LAPIC
- ACPI

## Future Features

- ACPI
- APIC
- PCI Driver
- Booting AP's
- Scheduler
- Disk driver

## Build

### Download:
 - Download source using `git clone`
### Requirements:
- NASM
- GCC (either system GCC or use a cross-compiler)
- GNU Linker
- QEMU
- GNU Make
- Limine
- Xorriso
- dd
### Run:
- Run with `make all`

### Caution:
I am not responsible for any damage to the hardware that you choose to run my code on. By running this code, you are consciously aware of the associated risks.
