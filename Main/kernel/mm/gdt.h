#ifndef GDT_H
#define GDT_H

#include <stdint.h>
#include <stddef.h>
#include "../lib/util.h"

#define GDT_MAX_ENTRIES 256

struct GDT_Descriptor {
    uint16_t limit0;
    uint16_t base0;
    uint8_t base1;
    uint8_t access;
    uint8_t granularity; // part of the limit stored here (bits 19:16)
    uint8_t base2;
} __attribute__((packed));

struct GDTR {
    uint16_t length;
    uint64_t base;
} __attribute__((packed));

struct GDT_Descriptor gdt[GDT_MAX_ENTRIES];

void gdt_init();

// Externally-linked functions
extern void gdt_reset_segment_registers(uint16_t cs, uint16_t data_seg_registers);
extern void __lgdt(uint64_t gdtr);

#endif // GDT_H