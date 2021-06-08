#include "gdt.h"

static inline void lgdt(uint64_t base, uint16_t size) {   // This function works only in 64-bit mode
    struct GDTR gdtr= { size, base };
    asm ( "lgdt %0" : : "m"(gdtr) );
}

void gdt_set_entry(uint8_t entry, uint16_t limit0, uint16_t base0, uint8_t base1, uint16_t type_attr, uint8_t base2) {
    gdt[entry] = (struct GDT_Descriptor) {limit0, base0, base1, type_attr, base2};
}

void gdt_init() {
    // @TODO @FIXME: ADD TSS ENTRY THIGNIE
    //            entry,     limit0         base0       base1       type_attr     base2
    gdt_set_entry(0,        0x0000,        0x0000,      0x00,       0x0000,       0x00);    // Null descriptor
    gdt_set_entry(1,        0xffff,        0x0000,      0x00,       0xAF98,       0x00);    // GDT64 Kernel Code
    gdt_set_entry(2,        0xffff,        0x0000,      0x00,       0x8F92,       0x00);    // GDT64 Kernel Data
    gdt_set_entry(3,        0xffff,        0x0000,      0x00,       0x8FF2,       0x00);    // GDT64 User Data
    gdt_set_entry(4,        0xffff,        0x0000,      0x00,       0xAFF8,       0x00);    // GDT64 User Code
    serial_puts("\n[GDT] Loading new GDT with address ");
    serial_puts(unsigned_long_to_str(gdt));
    serial_puts(" and limit ");
    serial_puts(unsigned_long_to_str(64*5 - 1));

    // Update the GDT with the new GDT
    lgdt(gdt, 64*5 - 1);
    serial_puts("\n - GDT loaded... initializing segment registers...");
    gdt_reset_segment_registers(0x8, 0x10);
    serial_puts("\n - Segment registers intialized!");
}