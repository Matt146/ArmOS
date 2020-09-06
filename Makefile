kernel.o : kernel.c
	gcc -ffreestanding -m32 -c kernel.c -o kernel.o

kernel.bin : kernel.o
	ld -o kernel.bin -Ttext 0x1000 kernel.o --oformat binary

boot.bin : boot.asm
	nasm -f bin boot.asm -o boot.bin

clean:
	rm *.bin *.oformat

all : kernel.bin boot.bin

run : all
	qemu-system-x86_64 os-image -no-reboot -monitor stdio -d int -D qemu.log -no-shutdown

os-image : boot.bin kernel.bin
	cat boot.bin kernel.bin > os_image