import os

print("[+] Building bootloader")
os.system("nasm -f bin boot.asm -o boot")
print("[+] Booting...")
os.system("qemu-system-x86_64 boot -no-reboot -monitor stdio -d int -D qemu.log -no-shutdown")
