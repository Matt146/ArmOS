#include "driver/vga.h"
#include "int/idt.h"
#include "mem/pmm.h"
#include "util/io.h"

__attribute__((section(".text")))
int kmain() {
        // Make sure interrupts are enabled
        __asm__("sti\n");

        // disable a20 for the lols
        outb(0xee, 10);

        // Clear the terminal and display the welcome message
        term_cls();
        term_kprint("Welcome to MattOS!\n\n", 0x1F);

        // Read the memory map
        read_bios_memory_map();
        
        // Initialize interrupts
        term_kprint("Initializing interrupts...\n", 0x1F);
        init_idt();

        // Kernel map some pages for the hell of it, ezzz
        term_kprint("Mapping pages...\n", 0x1F);
        for (uint64_t i = 0; i < 2; i++) {
                unsigned char* mem = (unsigned char*)kmalloc();
        }
        term_kprint("MAPPING WORKED!\n", 0x1F);

        __asm__("cli\nhlt\n");
}