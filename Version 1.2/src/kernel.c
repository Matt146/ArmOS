#include "driver/vga/vga.h"
#include "driver/etc/io.h"
#include "mem/vmm.h"
#include "mem/pmm.h"
#include "int/isr.h"

__attribute__((section(".text")))
int kmain() {
    // Do some hello world print shit
    vga_cls(VGA_COLOR_RED);
    for (size_t i = 0; i < 2000; i++) {
        vga_putc('A' + (i % 42), VGA_COLOR_WHITE << 4 | VGA_COLOR_BLUE);
    }
    vga_cls(VGA_COLOR_GREEN);

    // Initialize interrupts
    vga_puts("[+] Initializing Interrupts...\n", VGA_COLOR_LIGHTBLUE);
    init_interrupts();
    vga_puts("[+] Interrupts Initialized!\n", VGA_COLOR_LIGHTBLUE);

    // Initialize BIOS memory map
    vga_puts("[+] Reading BIOS memory map...\n", VGA_COLOR_GREEN);
    pmm_read_bios_memory_map();
    vga_puts("[+] Read BIOS memory map!\n", VGA_COLOR_GREEN);
    pmm_init_bitmap();
    vga_puts("[+] Bitmap Nonzero Print:\n", VGA_COLOR_LIGHTMAGENTA);
    for (size_t i = 0; i < BITMAP_ENTRIES; i++) {
        if (pmm_bitmap[i] > 0) {
            vga_puts(unsigned_long_to_str(pmm_bitmap[i]), VGA_COLOR_LIGHTMAGENTA);
        }
    }

    // DEBUG
    // DEBUG();

    // vga_print(str, 15, VGA_COLOR_WHITE << 4 | VGA_COLOR_BLUE);
    // Busy loop
    while (1 == 1) {

    }
    return 0;
}