# Boot Source Files & Outputs
boot_asm = src/boot/boot.asm
# Kernel Source Files & Outputs
kernel_sources_c = src/kernel.c src/driver/vga/vga.c src/int/isr.c src/mem/pmm.c src/mem/vmm.c src/driver/e9/e9.c src/driver/serial/serial.c src/int/lapic.c
kernel_obj_c = kernel.o vga.o isr.o pmm.o vmm.o e9.o  serial.o lapic.o
kernel_sources_asm = src/driver/etc/io_asm.asm src/int/idt_asm.asm src/mem/mem_asm.asm
kernel_obj_asm = io_asm.o idt_asm.o mem_asm.o

# Disk Image
boot_bin = boot.bin
kernel_bin = kernel.bin
img = bin/img.iso

# Linker Script Stuff
link = linker.ld

CC = gcc
CFLAGS = -c -O1 -Wall -Wextra -fno-stack-protector -fno-exceptions -fno-pic -mno-sse -mno-sse2 -mno-mmx -mno-80387 -mno-red-zone -g0 -fno-omit-frame-pointer -nostdlib -m64
#CFLAGS = -c -m64 -ffreestanding -nostdlib -mno-red-zone -fno-pic

ASM = nasm -f elf64

kernel:
	# Compile the kernel C sources
	$(CC) $(CFLAGS) $(kernel_sources_c)
	# Compile the kernel asm sources
	$(ASM) src/driver/etc/io_asm.asm -o io_asm.o
	$(ASM) src/int/idt_asm.asm -o idt_asm.o
	$(ASM) src/mem/mem_asm.asm -o mem_asm.o
	# Link the kernel
	ld -o $(kernel_bin) -T linker.ld  $(kernel_obj_c) $(kernel_obj_asm) --oformat binary

bootloader:
	# Compile the bootloader
	nasm -f bin $(boot_asm) -o $(boot_bin)

patch:
	# img.iso = bootloader + kernel_compiled
	cat $(boot_bin) $(kernel_bin) > $(img)

pad:
	# Pad the ISO to 32 KB
	dd if=/dev/zero bs=1 count=32768 >> $(img)

all: bootloader kernel patch pad
	# Move everything into the bin folder
	mv *.o bin/
	mv *.bin bin/
	# Boot the OS in QEMU
	qemu-system-x86_64 $(img) -no-reboot -monitor stdio -d int -D debug/qemu.log -no-shutdown -serial file:'debug/COM1.log' -m 4G -smp 4

clean:
	rm -rf debug/*
	rm -rf *.o
	rm -rf bin/*
	rm -rf *.bin
	touch debug/qemu.log
