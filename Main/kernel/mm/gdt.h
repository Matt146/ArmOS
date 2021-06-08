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
    uint16_t type_attr; // part of the limit stored here (bits 19:16)
    uint8_t base2;
} __attribute__((packed));

struct GDTR {
    uint16_t length;
    uint64_t base;
} __attribute__((packed));

struct GDT_Descriptor gdt[GDT_MAX_ENTRIES];

void gdt_set_entry(uint8_t entry, uint16_t limit0, uint16_t base0, uint8_t base1, uint16_t type_attr, uint8_t base2);
void gdt_init();
static inline void lgdt(uint64_t base, uint16_t size);

// Externally-linked functions
extern void gdt_reset_segment_registers(uint16_t cs, uint16_t data_seg_registers);

#endif // GDT_H