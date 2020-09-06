import os

print("[+] Building bootloader... nasm -f bin boot.asm -o boot")
os.system("nasm -f bin boot.asm -o boot.bin")
print("[+] Compiling kernel... make")
os.system("nasm -f bin kernel_entry.asm -o kernel_entry.bin")
os.system("cat boot.bin kernel_entry.bin > image.iso")
print("[+] Running... qemu-system-x86_64 os-image -no-reboot -monitor stdio -d int -D qemu.log -no-shutdown")
os.system("qemu-system-x86_64 image.iso -no-reboot -monitor stdio -d int -D qemu.log -no-shutdown")
#print("[+] Booting... qemu-system-x86_64 boot -no-reboot -monitor stdio -d int -D qemu.log -no-shutdown")
#os.system("qemu-system-x86_64 boot -no-reboot -monitor stdio -d int -D qemu.log -no-shutdown")

# nasm -f bin boot.asm -o boot.bin
# gcc -ffreestanding -c kernel.c -o kernel.o
# ld -o kernel.bin -Ttext 0x1000 kernel_entry.o kernel.o --oformat binary
# cat boot.bin kernel.bin > os-image
