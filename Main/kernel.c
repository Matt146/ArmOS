#include <stdint.h>
#include <stddef.h>
#include <stivale.h>
#include "kernel/driver/serial/serial.h"
#include "kernel/sched/mutex.h"
#include "kernel/acpi/rsdp.h"
#include "kernel/acpi/madt.h"
#include "kernel/int/lapic.h"
#include "kernel/sched/mp.h"
#include "kernel/lib/util.h"
#include "kernel/int/idt.h"
#include "kernel/mm/slab.h"
#include "kernel/mm/pmm.h"
#include "kernel/mm/gdt.h"

// We need to tell the stivale bootloader where we want our stack to be.
// We are going to allocate our stack as an uninitialised array in .bss.
static uint8_t stack[4096];

// The stivale specification says we need to define a "header structure".
// This structure needs to reside in the .stivalehdr ELF section in order
// for the bootloader to find it. We use this __attribute__ directive to
// tell the compiler to put the following structure in said section.
__attribute__((section(".stivalehdr"), used))
static struct stivale_header stivale_hdr = {
    // Let's tell the bootloader where our stack is.
    // We need to add the sizeof(stack) since in x86(_64) the stack grows
    // downwards.
    .stack = (uintptr_t)stack + sizeof(stack),
    // These flags can be used to tell the bootloader to enable or disable
    // certain features. We turn on bit 0 as we are going to ask for a
    // graphical framebuffer. Not setting this bit will make the
    // bootloader default to a CGA-compatible text mode instead.
    // We also toggle bit 3 as that will make the bootloader offset pointers
    // given to us to the higher half.
    .flags = (1 << 0) | (1 << 3),
    // We set all the framebuffer specifics to 0 as we want the bootloader
    // to pick the best it can.
    .framebuffer_width  = 0,
    .framebuffer_height = 0,
    .framebuffer_bpp    = 0,
    // The entry_point member is used to specify an alternative entry
    // point that the bootloader should jump to instead of the executable's
    // ELF entry point. We do not care about that so we leave it zeroed.
    .entry_point = 0
};

// The following will be our kernel's entry point.
void _start(struct stivale_struct *stivale_struct) {
    serial_puts("[LOG] Starting kernel...");

    // Initialize the IDT
    idt_init();

    // Initialize serial
    serial_init();

    asm volatile ("sti");

    // Initialize the PMM and perform a test alloc
    srand(56);
    pmm_init(stivale_struct);
    uint64_t alloc = pmm_alloc(25);
    serial_puts("\n[LOG] ALLOC BLOCK: ");
    serial_puts(unsigned_long_to_str(pmm_paddr_to_block(alloc)));
    pmm_free(alloc, 25);

    // PMM Test Operations
    /*
    for (size_t i = 0; i < 10; i++) {
        uint64_t rand_num = rand64() % 10;
        serial_puts("\n[LOG] Allocating ");
        serial_puts(unsigned_long_to_str(rand));
        serial_puts(" blocks");
        uint64_t alloc = pmm_alloc(rand_num);
        serial_puts("\n - ALLOC BLOCK: ");
        serial_puts(unsigned_long_to_str(pmm_paddr_to_block(alloc)));
    }
    */

    // Initialize the VMM
    vmm_init();

    // VMM Test Operations
    bool mapped = vmm_page_is_mapped(0xffff800000000000);
    if (mapped) {
        serial_puts("\n[LOG] Testing vmm - page mapped. CHECK!");
    } else {
        serial_puts("\n[LOG] Testing vmm - page unmapped. FAILED!");
    }
    /* VMM Unmap Test
    vmm_unmap_page(0xffff800000000000);
    vmm_flush_cr3();
    mapped = vmm_page_is_mapped(0xffff800000000000);
    if (mapped) {
        serial_puts("\n[LOG] Testing vmm - page mapped. CHECK!");
    } else {
        serial_puts("\n[LOG] Testing vmm - page unmapped. FAILED!");
    }
    */

   // Initialize slab allocator
   mm_slab_init();
   serial_puts("\n[LOG] INITIALIZED SLAB ALLOCATOR!");
   mm_slab_debug();

    // Test kmalloc
    for (size_t i = 0; i < 10; i++) {
        void* data = kmalloc(64);
        serial_puts("\n[LOG] KMALLOC ADDR: ");
        serial_puts(unsigned_long_to_str((uint64_t)data));
    }


    // Initialize the new GDT
    gdt_init();

    // Initialize ACPI
    acpi_init(stivale_struct);
    uint64_t madt_addr = acpi_find_table(ACPI_MADT);
    serial_puts("\n[LOG] ACPI MADT found: ");
    serial_puts(unsigned_long_to_str(madt_addr));
    acpi_madt_detect_cores(madt_addr);

    // Initialize the lapic
    lapic_init();

    // Initialize MP
    mp_init();

    /*
    // Mutex Test Operations
    serial_puts("\n[INFO] Locking mutex...");
    uint8_t mu;
    mutex_lock(&mu);
    serial_puts("\n[INFO] Mutex locked.");
    mutex_unlock(&mu);
    serial_puts("\n[INFO] Mutex unlocked!");
    */

    // Initialize the lapic timer
    lapic_init_timer();
    //lapic_set_timer(0x100);

    // Let's get the address of the framebuffer.
    uint8_t *fb_addr = (uint8_t *)stivale_struct->framebuffer_addr;

    // Let's try to paint a few pixels white in the top left, so we know
    // that we booted correctly.
    for (size_t i = 0; i < 128; i++) {
        fb_addr[i] = 0xff;
    }

    serial_puts("\n[LOG] Done execution. Halting...\n");

    // We're done, just hang...
    for (;;) {
        asm ("hlt");
    }
}
