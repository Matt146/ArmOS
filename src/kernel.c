#include "driver/vga.h"
#include "int/idt.h"

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

        // Test interrupt
        term_kprint("Testing interrupts...\n", 0x1F);
        __asm__("int $0x80\n");
        term_kprint("TEST SUCCESS!\n", 0x1F);

        __asm__("cli\nhlt\n");
}