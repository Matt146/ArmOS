#include "ioapic.h"

uint64_t ioapic_get_base(size_t ioapic) {
    ioapics[ioapic].ioapic_base = acpi_ioapic[ioapic].ioapic_addr;
    return acpi_ioapic[ioapic].ioapic_addr;
}

uint32_t ioapic_get_gsi(size_t ioapic) {
    ioapics[ioapic].ioapic_gsi = acpi_ioapic[ioapic].global_system_interrupt_base;
    return ioapics[ioapic].ioapic_gsi;
}

uint32_t ioapic_get_max_redirection_entry(size_t ioapic) {
    ioapics[ioapic].ioapic_max_redirection_entry = (ioapic_read(ioapic, 1) & 0xff0000) >> 16;
    return ioapics[ioapic].ioapic_max_redirection_entry;
}

void ioapic_init() {

    // Get GSI, base, and max redirection entry
    for (size_t i = 0; i < acpi_ioapics_count; i++) {
        // For each ioapic print debug information
        ioapic_get_base(i);
        ioapic_get_gsi(i);
        ioapic_get_max_redirection_entry(i);

        // Pritn debug infroamtion
        serial_puts("\n[IOAPIC] base: ");
        serial_puts(unsigned_long_to_str((uint64_t)ioapics[i].ioapic_base));
        serial_puts("\n[IOAPIC] gsi: ");
        serial_puts(unsigned_long_to_str((uint64_t)ioapics[i].ioapic_gsi));
        serial_puts("\n[IOAPIC] max redirection entry: ");
        serial_puts(unsigned_long_to_str((uint64_t)ioapics[i].ioapic_max_redirection_entry));
    }

    // Map IRQ's now lessgoooo
    for (size_t x = 0; x < acpi_ioapics_count; x++) {
        for (uint8_t i = ioapics[x].ioapic_gsi; i < ioapics[x].ioapic_max_redirection_entry + ioapics[x].ioapic_gsi + 1; i++) {
            serial_puts("\n[IOAPIC] Remapping IOAPIC Stuff...");
            ioapic_map_irq(x, i, i, false, lapic_get_current_id());
        }
    }
}

static uint32_t ioapic_read(size_t ioapic, uint32_t reg) {
    volatile uint32_t *base = ioapics[ioapic].ioapic_base;
    *base = reg;
    return *(base + 4);
}

static void ioapic_write(size_t ioapic, uint32_t reg, uint32_t data) {
    volatile uint32_t *base = (volatile uint32_t*)ioapics[ioapic].ioapic_base;
    *base = reg;
    *(base + 4) = data;
}

void ioapic_map_irq(size_t ioapic, uint8_t irq, uint8_t vector, bool masked, uint8_t destination) {
    if (irq > ioapics[ioapic].ioapic_gsi && irq < ioapics[ioapic].ioapic_max_redirection_entry + ioapics[ioapic].ioapic_gsi + 1) {
        serial_puts("\n2. [IOAPIC] Remapping IOAPIC Stuff...");
        uint8_t ioregsel_low32 = 0x10 + vector * 2;
        uint8_t ioregsel_high32 = 0x10 + vector * 2 + 1;
        // Set low32
        if (masked == false) {
            ioapic_write(ioapic, ioregsel_low32, vector);
        } else {
            ioapic_write(ioapic, ioregsel_low32, vector | (1 << 16));
        }

        ioapic_write(ioapic, ioregsel_high32, destination << 24);
    }
}