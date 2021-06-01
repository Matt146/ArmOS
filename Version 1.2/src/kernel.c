#include "driver/vga/vga.h"
#include "driver/etc/io.h"
#include "mem/vmm.h"
#include "mem/pmm.h"
#include "int/isr.h"
#include "int/lapic.h"
#include "driver/serial/serial.h"

__attribute__((section(".text")))
int kmain() {
    __asm__ volatile("sti\r\n");
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
    
    // Initialize serial
    init_serial();
    serial_puts("Hello, world! - TESTING SERIAL\n"); 

    // Initialize BIOS memory map and PMM Bitmap
    vga_puts("[+] Reading BIOS memory map...\n", VGA_COLOR_GREEN);
    pmm_read_bios_memory_map();
    vga_puts("[+] Read BIOS memory map!\n", VGA_COLOR_GREEN);
    pmm_init_bitmap();
    serial_puts("[+] Bitmap Nonzero Print:\n");
    for (size_t i = 0; i < BITMAP_ENTRIES; i++) {
        if (pmm_bitmap[i] > 0) {
            serial_puts(unsigned_long_to_str(pmm_bitmap[i]));
        }
    }

    vmm_init();
    vmm_map_vaddr_to_paddr(0x100000000, 0x100000000, 0x3);
    // Busy loop and then clear the screen
    /*
    vga_cls(VGA_COLOR_MAGENTA);
    vga_puts(unsigned_long_to_str(pmm_alloc(4)), VGA_COLOR_MAGENTA);
    vga_puts("\n", VGA_COLOR_MAGENTA);
    vga_puts(unsigned_long_to_str(pmm_alloc(4)), VGA_COLOR_MAGENTA);
    vga_puts("\n", VGA_COLOR_MAGENTA);
    */
    /*
    vga_cls(VGA_COLOR_MAGENTA);
    uint64_t alloc = pmm_alloc(10);
    vga_puts(unsigned_long_to_str(alloc), VGA_COLOR_MAGENTA);
    pmm_free(alloc, 10);
    */

    /*
    vga_puts(unsigned_long_to_str(pmm_block_is_free(63)), VGA_COLOR_MAGENTA);
    vga_puts("\n", VGA_COLOR_MAGENTA);
    */

    // DEBUG
    // DEBUG();

    // vga_print(str, 15, VGA_COLOR_WHITE << 4 | VGA_COLOR_BLUE);
    // Busy loop
    uint64_t lapic_base = pmm_alloc(1);
    serial_puts("\n[+]LAPIC BASE ADDRESS: ");
    serial_puts(unsigned_long_to_str(lapic_base));
    lapic_remap((uint32_t)lapic_base);
    serial_puts("\n[+]LAPIC remapped!");
    lapic_enable();
    serial_puts("\n[+]LAPIC enabled!");
    serial_puts("\n[+]Value of IA32_APIC_BASE MSR: ");
    serial_puts(unsigned_long_to_str(rdmsr(0x1B)));
    lapic_lvt_init_timer();
    serial_puts("\n[+] Initialized LAPIC timer...");
    lapic_timer_set(100);
    serial_puts("\n[+] LAPIC Error Register Dump:");
    serial_puts(unsigned_long_to_str(*((uint32_t*)lapic_base + 0x280)));
    serial_puts("\n[+] RFLAGS VALUE:");
    serial_puts(unsigned_long_to_str(VIEW_RFLAGS()));


    while (1 == 1) {

    }
    return 0;
}
