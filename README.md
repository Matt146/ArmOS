# ArmOS

This is an OS that I'm writing myself totally from scratch.

## Current progress

- Entered Long Mode
- Interrupts/IDT
- Serial COM1
- PMM
- VMM
- GDT
- LAPIC Timer
- LAPIC IPI's
- ACPI: RDSP, RSDT, XSDT, MADT

## WIP:

- VMM: Unmapping pages & checking if page is mapped
- Booting Cores

## Future Features

- IOAPIC
- PCI Driver
- Scheduler
- Disk driver

## TODO (to learn):
- Finish reviewing x86 protections and stack switching
- Refresh sycall knowledge
- Learn about booting cores, multiprocessing, task switching, multithreading, etc
- Memorize LAPIC stuff
- Memorize ACPI stuff

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
