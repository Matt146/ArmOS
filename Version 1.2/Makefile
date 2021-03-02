# Boot Source Files & Outputs
boot_asm = src/boot/boot.asm
# Kernel Source Files & Outputs
kernel_sources_c = src/kernel.c src/driver/vga/vga.c src/int/isr.c
kernel_obj_c = kernel.o vga.o isr.o
kernel_sources_asm = src/driver/etc/io_asm.asm
kernel_obj_asm = io_asm.o

# Disk Image
boot_bin = boot.bin
kernel_bin = kernel.bin
img = bin/img.iso

# Linker Script Stuff
link = linker.ld

kernel:
	# Compile the kernel C sources
	gcc -c -m64 -mno-red-zone $(kernel_sources_c)
	# Compile the kernel asm sources
	nasm -f elf64 src/driver/etc/io_asm.asm -o io_asm.o
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
	# Boot the OS in QEMU
	qemu-system-x86_64 $(img) -no-reboot -monitor stdio -d int -D debug/qemu.log -no-shutdown

clean:
	rm -rf debug/*
	rm -rf *.o
	rm -rf bin/*
	rm -rf *.bin
	touch debug/qemu.log