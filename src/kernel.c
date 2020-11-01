__asm__("jmp kmain\n\t");

#include "driver/vga.h"
#include "int/idt.h"

int kmain() {
        // Clear the terminal and display the welcome message
        term_cls();
        term_kprint("Welcome to MattOS!\n\n", 0x1F);
        
        // Initialize interrupts
        term_kprint("Initializing interrupts...\n", 0x1F);
        __asm__("cli\nhlt\n");
}