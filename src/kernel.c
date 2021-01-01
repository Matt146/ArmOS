#include "driver/vga.h"
#include "int/idt.h"
#include "mem/kmalloc.h"

__attribute__((section(".text")))
int kmain() {
        // Make sure interrupts are enabled
        __asm__("sti\n");

        // Clear the terminal and display the welcome message
        term_cls();
        term_kprint("Welcome to MattOS!\n\n", 0x1F);
        
        // Initialize interrupts
        term_kprint("Initializing interrupts...\n", 0x1F);
        init_idt();

        // Kernel map some pages for the hell of it, ezzz
        term_kprint("Mapping pages...\n", 0x1F);
        for (size_t i = 0; i < 2; i++) {
                unsigned char* mem = (unsigned char*)kmalloc();
        }
        term_kprint("MAPPING WORKED!\n", 0x1F);

        /*
        // Test interrupt
        term_kprint("Testing interrupts...\n", 0x1F);
        int x = 1;
        int y = 0;
        int z = 1 / 0;
        //__asm__("int $0x80\n");
        term_kprint("TEST SUCCESS!\n", 0x1F);
        */

        __asm__("cli\nhlt\n");
}