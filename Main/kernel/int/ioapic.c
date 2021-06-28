#include "ioapic.h"

uint64_t ioapic_get_base() {
    ioapic_base = acpi_ioapic.ioapic_addr;
    return ioapic_base;
}

uint32_t ioapic_get_gsi() {
    ioapic_gsi = acpi_ioapic.global_system_interrupt_base;
    return ioapic_gsi;
}

uint32_t ioapic_get_max_redirection_entry() {
    ioapic_max_redirection_entry = (ioapic_read(1) & 0xff0000) >> 16;
    return ioapic_max_redirection_entry;
}

void ioapic_init() {
    // Get GSI, base, and max redirection entry
    ioapic_get_gsi();
    ioapic_get_base();
    ioapic_get_max_redirection_entry();

    // Print debug information
    serial_puts("\n[IOAPIC] base: ");
    serial_puts(unsigned_long_to_str((uint64_t)ioapic_base));
    serial_puts("\n[IOAPIC] gsi: ");
    serial_puts(unsigned_long_to_str((uint64_t)ioapic_gsi));
    serial_puts("\n[IOAPIC] max redirection entry: ");
    serial_puts(unsigned_long_to_str((uint64_t)ioapic_max_redirection_entry));

    // Map IRQ's now lessgoooo
    for (uint8_t i = ioapic_gsi; i < ioapic_max_redirection_entry + ioapic_gsi + 1; i++) {
        ioapic_map_irq(i, i, false, lapic_get_current_id());
    }
}

static uint32_t ioapic_read(uint32_t reg) {
    volatile uint32_t *base = ioapic_base;
    *base = reg;
    return *(base + 4);
}

static void ioapic_write(uint32_t reg, uint32_t data) {
    volatile uint32_t *base = ioapic_base;
    *base = reg;
    *(base + 4) = data;
}

void ioapic_map_irq(uint8_t irq, uint8_t vector, bool masked, uint8_t destination) {
    if (irq > ioapic_gsi && irq < ioapic_max_redirection_entry + ioapic_gsi + 1) {
        uint8_t ioregsel_low32 = 0x10 + vector * 2;
        uint8_t ioregsel_high32 = 0x10 + vector * 2 + 1;
        // Set low32
        if (masked == false) {
            ioapic_write(ioregsel_low32, vector);
        } else {
            ioapic_write(ioregsel_low32, vector | (1 << 16));
        }

        ioapic_write(ioregsel_high32, destination << 24);
    }
}