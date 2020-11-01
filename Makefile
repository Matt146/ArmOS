# Kernel source file variables
kernel_sources_c = src/kernel.c src/driver/vga.c src/util/bits.c src/int/idt.c
kernel_sources_asm = src/int/isr.asm
kernel_bin = build/kernel.bin

# Kernel object files
kernel_obj_c = kernel.o vga.o bits.o idt.o
kernel_obj_asm = isr.o

all:
	# Compile the kernel C sources
	gcc -ffreestanding -c $(kernel_sources_c)
	# Compile the kernel asm sources
	nasm -f bin $(kernel_sources_asm) -o $(kernel_obj_asm)
	# Link the kernel
	ld -o $(kernel_bin) -T linker.ld  $(kernel_obj_c) $(kernel_obj_asm) --oformat binary
	# Compile the bootloader
	nasm -f bin src/boot.asm -o build/boot.bin
	cat build/boot.bin build/kernel.bin > build/img.iso
	# Pad the ISO to 32 KB
	dd if=/dev/zero bs=1 count=32768 >> build/img.iso
	# Boot the OS in QEMU
	qemu-system-x86_64 build/img.iso -no-reboot -monitor stdio -d int -D debug/qemu.log -no-shutdown

clean:
	# Clean up vim save/undo files	
	find . -type f -name '.*.un~' -delete
	find . -type f -name '*~' -delete
	# Clean up bootloader and kernel obj file now, so everything left should just be an object file
	find build -type f -name '*.bin' -delete
	find build -type f -name '*.o' -delete
