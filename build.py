import os

print("[+] Building bootloader")
os.system("nasm -f bin src/boot.asm -o build/boot")
os.system("nasm -f bin src/kernel_entry.asm -o build/kernel_entry")
os.system("cat build/boot build/kernel_entry > build/img.iso")
print("[+] Booting...")
os.system("qemu-system-x86_64 build/img.iso -no-reboot -monitor stdio -d int -D debug/qemu.log -no-shutdown")
