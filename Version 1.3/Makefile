KERNEL := stivale.elf

ASM = nasm
CC = gcc

CFLAGS = -Wall -Wextra -O2 -pipe

INTERNALLDFLAGS := \
	-fno-pic -fpie \
	-Wl,-static,-pie,--no-dynamic-linker,-ztext \
	-static-pie    \
	-nostdlib      \
	-Tlinker.ld    \
	-z max-page-size=0x1000

INTERNALCFLAGS  :=       \
	-I.                  \
	-std=gnu11           \
	-ffreestanding       \
	-fno-stack-protector \
	-fno-pic -fpie       \
	-mno-80387           \
	-mno-mmx             \
	-mno-3dnow           \
	-mno-sse             \
	-mno-sse2            \
	-mno-red-zone

INTERNALASMFLAGS := -f elf64

CFILES := $(shell find ./ -type f -name '*.c')
ASMFILE := kernel/lib/asm_common.asm
OBJ    := $(CFILES:.c=.o)
ASMOBJ := kernel/lib/asm_common.o

.PHONY: all clean

all: $(KERNEL) image.iso

$(KERNEL): $(OBJ)
	$(ASM) $(ASMFILE) $(INTERNALASMFLAGS) -o $(ASMOBJ)
	$(CC) $(INTERNALLDFLAGS) $(OBJ) $(ASMOBJ) -o $@

stivale.h:
	wget https://github.com/stivale/stivale/raw/master/stivale.h

%.o: %.c stivale.h
	$(CC) $(CFLAGS) $(INTERNALCFLAGS) -c $< -o $@

image.iso: stivale.h $(KERNEL)
	git clone https://github.com/limine-bootloader/limine.git --branch=v2.0-branch-binary --depth=1
	make -C limine
	mkdir -p iso_root
	cp -v stivale.elf limine.cfg limine/limine.sys \
      		limine/limine-cd.bin limine/limine-eltorito-efi.bin iso_root/
	xorriso -as mkisofs -b limine-cd.bin \
        	-no-emul-boot -boot-load-size 4 -boot-info-table \
        	--efi-boot limine-eltorito-efi.bin \
        	-efi-boot-part --efi-boot-image --protective-msdos-label \
        	iso_root -o image.iso
	./limine/limine-install image.iso
	qemu-system-x86_64 image.iso -no-reboot -monitor stdio -d int -D debug/qemu.log -no-shutdown -serial file:'debug/COM1.log' -m 8G -smp 4

clean:
	rm -rf $(KERNEL) $(OBJ) $(ASMOBJ)
