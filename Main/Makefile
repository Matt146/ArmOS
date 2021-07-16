KERNEL := armos.elf

ASM = nasm
CC = gcc

CFLAGS = -Wall -Wextra -O2 -pipe

INTERNALLDFLAGS := 	-no-pie \
				   	-nostdlib \
					-nodefaultlibs \
					-nostartfiles \
				    -z max-page-size=0x1000

INTERNALCFLAGS  :=	-I. \
					-std=gnu11 \
					-ffreestanding \
					-fno-stack-protector \
					-fno-pic -fpie \
					-mno-80387 \
					-mno-mmx \
					-mno-3dnow \
					-mno-sse \
					-mno-sse2 \
					-mno-red-zone \
					-fcommon

QEMUFLAGS :=	-vga std \
				-serial file:'com1.log' \
				-m 4G \
				-smp 4 \
				-drive file=disk.img,if=none,id=NVME1 \
				-device nvme,drive=NVME1,serial=nvme \
				-drive id=disk,file=armos.img,if=none \
				-device ahci,id=ahci \
				-device ide-hd,drive=disk,bus=ahci.0

INTERNALASMFLAGS := -f elf64

CFILES := $(shell find ./ -type f -name '*.c' ! -name 'limine-install.c' ! -name 'boot.asm')
ASMFILE := ./kernel/lib/asm_common.asm
OBJ    := $(CFILES:.c=.o)
ASMOBJ := ./kernel/lib/asm_common.o

.PHONY: all clean

all: clean build

stub:
	nasm -f bin ./kernel/sched/boot.asm -o ./kernel/sched/boot.bin

$(KERNEL): $(OBJ) stub
	$(ASM) $(ASMFILE) $(INTERNALASMFLAGS) -o $(ASMOBJ)
	nasm -f elf64 ./kernel/sched/real.asm -o ./kernel/sched/real.o
	$(CC) $(INTERNALLDFLAGS) $(OBJ) $(ASMOBJ) ./kernel/sched/real.o -o $@

stivale.h:
	wget https://github.com/stivale/stivale/raw/master/stivale.h

%.o: %.c stivale.h
	$(CC) $(CFLAGS) $(INTERNALCFLAGS) -c $< -o $@

limine:
	git clone https://github.com/limine-bootloader/limine.git --single-branch -b latest-binary

build: stivale.h $(KERNEL) limine
	rm -f armos.img disk.img
	dd if=/dev/zero bs=1M count=0 seek=128 of=armos.img
	dd if=/dev/zero bs=1M count=0 seek=128 of=disk.img
	parted -s armos.img mklabel msdos
	parted -s armos.img mkpart primary 1 100%
	rm -rf disk_image/
	mkdir disk_image
	sudo losetup -Pf --show armos.img > loopback_dev
	sudo mkfs.ext2 `cat loopback_dev`p1
	sudo mount `cat loopback_dev`p1 disk_image
	sudo mkdir disk_image/boot
	sudo cp armos.elf disk_image/boot/
	sudo cp limine.cfg disk_image/
	sudo cp limine/limine.sys disk_image/boot/
	sync
	sudo umount disk_image/
	sudo losetup -d `cat loopback_dev`
	rm -rf disk_image loopback_dev
	./limine/limine-install-linux-x86_64 armos.img
	parted -s disk.img mklabel msdos
	parted -s disk.img mkpart primary 1 100%
	sudo losetup -Pf --show disk.img > loopback_dev
	sudo mkfs.ext2 `cat loopback_dev`p1
	sudo losetup -d `cat loopback_dev`
	sync
	rm -rf loopback_dev

qemu: build
	touch com1.log
	qemu-system-x86_64 $(QEMUFLAGS) &
	tail -n0 -f com1.log

console: build
	qemu-system-x86_64 $(QEMUFLAGS) -no-reboot -monitor stdio -d int -D qemu.log -no-shutdown -vga vmware

debug: build
	qemu-system-x86_64 $(QEMUFLAGS) -no-reboot -monitor stdio -d int -no-shutdown

clean:
	rm -rf $(KERNEL) $(OBJ) $(ASMOBJ) ./kernel/sched/real.o ./kernel/sched/boot.bin disk.img armos.img com1.log qemu.log
