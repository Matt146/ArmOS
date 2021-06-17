#include "madt.h"

void acpi_madt_detect_cores(uint64_t madt_addr) {
    struct MADT* madt_header_ptr = (struct MADT*)madt_addr;
    uint8_t* cur_madt_ptr = 0x2C + madt_addr;
    uint64_t max_len = madt_header_ptr->header.length;

    serial_puts("\n[ACPI - MADT] MADT SDT Header:");
    serial_puts("\n - Signature: ");
    for (size_t i = 0; i < 4; i++) {
        serial_putc((char)madt_header_ptr->header.signature[i]);
    }
    serial_putc('\0');
    serial_puts("\n - Length: ");
    serial_puts(unsigned_long_to_str(madt_header_ptr->header.length));
    serial_puts("\n - Revision: ");
    serial_puts(unsigned_long_to_str(madt_header_ptr->header.revision));
    serial_puts("\n - Checksum: ");
    serial_puts(unsigned_long_to_str(madt_header_ptr->header.checksum));
    serial_puts("\n - OEMID: ");
    serial_puts(unsigned_long_to_str(madt_header_ptr->header.OEMID));


    size_t cur_lapic = 0;
    acpi_detected_processors_count = 0;
    while (true) {
        // Check if we reach the end
        if ((uint64_t)cur_madt_ptr >= max_len + madt_addr) {
            break;
        }

        if (*cur_madt_ptr == 0) {
            // Processor LAPIC
            struct MADT_EntryType0* madt_processor_lapic = (struct MADT_EntryType0*)cur_madt_ptr;
            acpi_processors[cur_lapic] = madt_processor_lapic->lapic_id;
            cur_madt_ptr += sizeof(struct MADT_EntryType0);
            cur_lapic += 1;
            acpi_detected_processors_count += 1;
            serial_puts("\n[ACPI - MADT] Detected Core: ");
            serial_puts(unsigned_long_to_str(madt_processor_lapic->lapic_id));
            continue;
        } else if (*cur_madt_ptr == 1) {
            // IOAPIC
            cur_madt_ptr += sizeof(struct MADT_EntryType1);
            continue;
        } else if (*cur_madt_ptr == 2) {
            // IOAPIC Interrupt Source Override
            cur_madt_ptr += sizeof(struct MADT_EntryType2);
            continue;
        } else if (*cur_madt_ptr == 3) {
            cur_madt_ptr += sizeof(struct MADT_EntryType3);
            continue;
        } else if (*cur_madt_ptr == 4) {
            cur_madt_ptr += sizeof(struct MADT_EntryType4);
            continue;
        } else if (*cur_madt_ptr == 5) {
            cur_madt_ptr += sizeof(struct MADT_EntryType5);
            continue;
        } else if (*cur_madt_ptr == 9) {
            cur_madt_ptr += sizeof(struct MADT_EntryType9);
            continue;
        }
    }
}