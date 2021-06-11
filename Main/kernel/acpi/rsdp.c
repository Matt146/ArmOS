#include "rsdp.h"

void acpi_init(struct stivale_struct* stivale_struct) {
    // Detect ACPI version
    serial_puts("\n[ACPI] Initializing ACPI");
    struct RSDP_Descriptor* rsdp_descriptor_ptr = (struct RSDP_Descriptor*)(stivale_struct->rsdp);
    if (rsdp_descriptor_ptr->revision == 0) {
        // ACPI version 1.0 - fucking trash
        serial_puts("\n - ACPI Version 1.0 detected");
        acpi_new = false;
        rsdt = (struct ACPI_RSDT*)(rsdp_descriptor_ptr->rsdt_addr);
        serial_puts("\n - ACPI RSDT Pointer: ");
        serial_puts(unsigned_long_to_str(rsdt));
    } else {
        // ACPI version 2.0 or above
        serial_puts("\n - ACPI Version 2.0 or above detected");
        acpi_new = true;
        struct RSDP_Descriptor_2* rsdp_descriptor_ptr2 = (struct RSDP_Descriptor_2*)(stivale_struct->rsdp);
        xsdt = (struct ACPI_XSDT*)(rsdp_descriptor_ptr2->xsdt_addr);
        serial_puts("\n - ACPI XSDT Pointer: ");
        serial_puts(unsigned_long_to_str(xsdt));
    }
}

bool acpi_validate_table(struct ACPI_SDT_Descriptor_Header* table_header) {
    uint64_t sum;
    for (size_t i = 0; i < table_header->length; i++) {
        sum += ((uint8_t*)table_header)[i];
    }
    if (sum == 0) {
        return true;
    }
    return false;
}

uint64_t acpi_find_table(char* table_signature) {
    if (acpi_new == false) {
        size_t total_rsdt_entries = (rsdt->header.length - sizeof(struct ACPI_SDT_Descriptor_Header)) / sizeof(uint32_t);
        for (size_t i = 0; i < total_rsdt_entries; i++) {
            uint32_t ptr = (uint32_t)(rsdt->table_ptrs)[i];
            struct ACPI_SDT_Descriptor_Header* table_ptr = (struct ACPI_SDT_Descriptor_Header*)ptr;
            if (strncmp(table_ptr->signature, table_signature, 4)) {
                return (uint64_t)table_ptr;
            }
        }
    } else {
        size_t total_xsdt_entries = (xsdt->header.length - sizeof(struct ACPI_SDT_Descriptor_Header)) / sizeof(uint64_t);
        for (size_t i = 0; i < total_xsdt_entries; i++) {
            uint64_t ptr = xsdt->table_ptrs[i];
            struct ACPI_SDT_Descriptor_Header* table_ptr = (struct ACPI_SDT_Descriptor_Header*)ptr;
            if (strncmp(table_ptr->signature, table_signature, 4)) {
                return (uint64_t)table_ptr;
            }
        }
    }

    return ACPI_TABLE_FIND_FAIL;
}