#ifndef IOAPIC_H
#define IOAPIC_H

///////////////////////////////////////////////////////////
// TO ANYONE READING THIS FILE - THERE MAY BE A BUG
// I ONLY CONSIDER 1 IOAPIC
// @FIXME LATER
///////////////////////////////////////////////////////////

#include <stdint.h>
#include <stddef.h>
#include "../acpi/madt.h"
#include "../mm/vmm.h"
#include "lapic.h"

static uint64_t ioapic_base;
static uint32_t ioapic_gsi;            // This is the first IRQ - Global System Interrupt
static uint8_t ioapic_max_redirection_entry;

void ioapic_init();
uint64_t ioapic_get_base();
uint32_t ioapic_get_gsi();
uint32_t ioapic_get_max_redirection_entry();

static void ioapic_write(uint32_t reg_offset, uint32_t value);
static uint32_t ioapic_read(uint32_t reg_offset);
void ioapic_map_irq(uint8_t irq, uint8_t vector, bool masked, uint8_t destination);

#endif // IOAPIC_H