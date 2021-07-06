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

bool ioapic_irq_overrided(uint8_t irq) {
    for (size_t i = 0; i < acpi_is_overrides_count; i++) {
        serial_puts("\n - irq ");
        serial_puts(unsigned_long_to_str((uint64_t)acpi_interrupt_source_overrides[i].irq_source));
        serial_puts(" --> gsi ");
        serial_puts(unsigned_long_to_str((uint64_t)acpi_interrupt_source_overrides[i].global_system_interrupt));
        serial_puts(" : ");
        serial_puts(unsigned_long_to_str((uint64_t)irq));
        if (irq == acpi_interrupt_source_overrides[i].irq_source || irq == acpi_interrupt_source_overrides[i].global_system_interrupt) {
            serial_puts("\n - RETURNING TRUE;");
            return true;
        }
    }
    return false;
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
            if (ioapic_irq_overrided(i) == false) {
                serial_puts("\n[IOAPIC] Remapping IOAPIC irq ");
                serial_puts(unsigned_long_to_str(ioapic_get_gsi_from_irq(i)));
                serial_puts(" to vector ");
                serial_puts(unsigned_long_to_str((uint64_t)ioapic_get_vector_from_irq(i)));
                if (ioapic_get_gsi_from_irq(i) == 2) {
                    ioapic_map_irq(x, ioapic_get_gsi_from_irq(i), ioapic_get_vector_from_irq(i), true, lapic_get_current_id());
                    continue;
                }
                ioapic_map_irq(x, ioapic_get_gsi_from_irq(i), ioapic_get_vector_from_irq(i), true, lapic_get_current_id());     // Set to false to unmask
            } else {
                for (size_t j = 0; j < acpi_is_overrides_count; j++) {
                    if (i == acpi_interrupt_source_overrides[j].irq_source) {
                        serial_puts("\n[IOAPIC] Remapping IOAPIC irq ");
                        serial_puts(unsigned_long_to_str(ioapic_get_gsi_from_irq(i)));
                        serial_puts(" to vector ");
                        serial_puts(unsigned_long_to_str((uint64_t)ioapic_get_vector_from_irq(i)));
                        if (ioapic_get_gsi_from_irq(i) == 2) {
                            ioapic_map_irq(x, ioapic_get_gsi_from_irq(i), ioapic_get_vector_from_irq(i), true, lapic_get_current_id());
                            continue;
                        }
                        ioapic_map_irq(x, ioapic_get_gsi_from_irq(i), ioapic_get_vector_from_irq(i), true, lapic_get_current_id()); // Set to false to unmask
                        break;
                    }
                }
            }
        }
    }
    serial_puts("\n[IOAPIC] Finished IRQ Mappings!");
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

uint8_t ioapic_get_vector_from_irq(uint8_t irq) {
    for (size_t i = 0; i < acpi_is_overrides_count; i++) {
        if (acpi_interrupt_source_overrides[i].irq_source == irq) {
            irq = acpi_interrupt_source_overrides[i].global_system_interrupt;   // set it to the corresponding global system interrupt
            return 254-irq;
        }
    }

    return irq + 32;
}

uint8_t ioapic_get_gsi_from_irq(uint8_t irq) {
     for (size_t i = 0; i < acpi_is_overrides_count; i++) {
        if (acpi_interrupt_source_overrides[i].irq_source == irq) {
            irq = acpi_interrupt_source_overrides[i].global_system_interrupt;   // set it to the corresponding global system interrupt
            return irq;
        }
    }

    return irq;
}

void ioapic_map_irq(size_t ioapic, uint8_t irq, uint8_t vector, bool masked, uint8_t destination) {
    if (irq > ioapics[ioapic].ioapic_gsi && irq < ioapics[ioapic].ioapic_max_redirection_entry + ioapics[ioapic].ioapic_gsi + 1) {
        irq = ioapic_get_gsi_from_irq(irq);
        uint8_t ioregsel_low32 = 0x10 + irq * 2;
        uint8_t ioregsel_high32 = 0x10 + irq * 2 + 1;
        // Set low32
        if (masked == false) {
            ioapic_write(ioapic, ioregsel_low32, vector);
        } else {
            ioapic_write(ioapic, ioregsel_low32, vector | (1 << 16));
        }

        ioapic_write(ioapic, ioregsel_high32, destination << 24);
    }
}