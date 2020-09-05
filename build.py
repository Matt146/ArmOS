import os

print("[+] Building bootloader")
os.system("nasm -f bin boot.asm -o boot")
print("[+] Booting...")
os.system("qemu-system-x86_64 boot")
