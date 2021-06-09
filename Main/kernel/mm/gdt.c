#include "gdt.h"

void gdt_init() {
    // NULL descriptor
    gdt[0].limit0 = 0;
    gdt[0].base0 = 0;
    gdt[0].base1 = 0;
    gdt[0].access = 0;
    gdt[0].granularity = 0;
    gdt[0].base2 = 0;

    // Kernel Code64
    gdt[1].limit0 = 0;
    gdt[1].base0 = 0;
    gdt[1].base1 = 0;
    gdt[1].access = 0x9A;
    gdt[1].granularity = 0x20;
    gdt[1].base2 = 0;

    // Kernel Data64
    gdt[2].limit0 = 0;
    gdt[2].base0 = 0;
    gdt[2].base1 = 0;
    gdt[2].access = 0x96;
    gdt[2].granularity = 0;
    gdt[2].base2 = 0;

    // User Data64
    gdt[3].limit0 = 0;
    gdt[3].base0 = 0;
    gdt[3].base1 = 0;
    gdt[3].access = 0xF2;
    gdt[3].granularity = 0;
    gdt[3].base2 = 0;

    // User Code64
    gdt[4].limit0 = 0;
    gdt[4].base0 = 0;
    gdt[4].base1 = 0;
    gdt[4].access = 0xFA;
    gdt[4].granularity = 0x20;

    // Update the GDT with the new GDT
    struct GDTR gdtr = {sizeof(struct GDT_Descriptor) * 5 - 1, &gdt};;
    __lgdt(&gdtr);
    serial_puts("\n - GDT loaded... initializing segment registers...");
    gdt_reset_segment_registers(0x8, 0x10);
}