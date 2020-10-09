import os

print("[+] Cleaning up vim files...")

# Clean up vim permasave files
print("rm -r .*.un~")
os.system("rm -r .*.un~")

# Clean up vim undo files
print("rm -r *~")
os.system("rm -r *~")

# Clean up junk files from building
print("rm boot")
os.system("rm boot")
print("rm kernel_entry")
os.system("rm kernel_entry")   
