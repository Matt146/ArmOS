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
- Booting Cores
- Mutexes/Locking
- IOAPIC
- PS/2: Keyboard Driver (scancodes)
- PCI Driver

## WIP:
- Disk Driver: NVME
    - MSI

## Future Features

- NIC Driver
- PS/2: Mouse Driver
- Slab Allocator
- HPET
- VFS
- Scheduler
- Filesystems: EXT2
- Framebuffer/GUI

## TODO (to learn):
- Look into scheduling algos + design scheduler
- Look into VFS's

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
