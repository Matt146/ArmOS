import os

print("[+] Building bootloader")
os.system("nasm -f bin boot.asm -o boot")
os.system("nasm -f bin kernel_entry.asm -o kernel_entry")
os.system("cat boot kernel_entry > img.iso")
print("[+] Booting...")
os.system("qemu-system-x86_64 img.iso -no-reboot -monitor stdio -d int -D qemu.log -no-shutdown")
