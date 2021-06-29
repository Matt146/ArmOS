#ifndef IOAPIC_H
#define IOAPIC_H

///////////////////////////////////////////////////////////
// TO ANYONE READING THIS FILE - THERE MAY BE A BUG
// ON PIC PIN 1 - remains default value
// @FIXME - check with "info pic" on qemu
///////////////////////////////////////////////////////////

#include <stdint.h>
#include <stddef.h>
#include "../acpi/madt.h"
#include "../mm/vmm.h"
#include "lapic.h"

struct IOAPIC {
    uint64_t ioapic_base;
    uint32_t ioapic_gsi;     // This is the first IRQ - Global System Interrupt
    uint8_t ioapic_max_redirection_entry;
} __attribute__((packed));

struct IOAPIC ioapics[ACPI_IOAPIC_MAX];

void ioapic_init();
uint64_t ioapic_get_base(size_t ioapic);
uint32_t ioapic_get_gsi(size_t ioapic);
uint32_t ioapic_get_max_redirection_entry(size_t ioapic);

static void ioapic_write(size_t ioapic, uint32_t reg_offset, uint32_t value);
static uint32_t ioapic_read(size_t ioapic, uint32_t reg_offset);

uint8_t ioapic_get_vector_from_irq(uint8_t irq);
uint8_t ioapic_get_gsi_from_irq(uint8_t irq);
bool ioapic_irq_overrided(uint8_t irq);
void ioapic_map_irq(size_t ioapic, uint8_t irq, uint8_t vector, bool masked, uint8_t destination);

#endif // IOAPIC_H