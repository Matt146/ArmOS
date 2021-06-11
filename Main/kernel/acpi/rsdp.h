#ifndef RSDP_H
#define RSDP_H

#include <stdint.h>
#include <stddef.h>
#include "../../stivale.h"
#include "../lib/panic.h"
#include "../driver/serial/serial.h"
#include "../mm/pmm.h"

#define ACPI_MADT "APIC"
#define ACPI_FADT "FACP"
#define ACPI_SRAT "SRAT"
#define ACPI_SSDT "SSDT"

#define ACPI_TABLE_FIND_FAIL PMM_ALLOC_FAIL // Address returned by acpi_find_table if finding the table fails

static bool acpi_new;   // set to true if we are using acpi 2.0+, else set false
static struct ACPI_RSDT* rsdt = NULL;
static struct ACPI_XSDT* xsdt = NULL;

// Root System Descriptor Pointer
struct RSDP_Descriptor {
    char signature[8];
    uint8_t checksum;
    char OEMID[6];
    uint8_t revision;
    uint32_t rsdt_addr;
} __attribute__((packed));

struct RSDP_Descriptor_2 {
    struct RSDP_Descriptor first_part;

    uint32_t length;
    uint64_t xsdt_addr;
    uint8_t extended_checksum;
    uint8_t reserved[3];
} __attribute__((packed));

// ACPI General System Descriptor Table Header
struct ACPI_SDT_Descriptor_Header {
    char signature[4];
    uint32_t length;
    uint8_t revision;
    uint8_t checksum;
    char OEMID[6];
    char OEMTableID[8];
    uint32_t OEMRevision;
    uint32_t creator_id;
    uint32_t creator_revision;
} __attribute__((packed));

struct ACPI_RSDT {
    struct ACPI_SDT_Descriptor_Header header;
    uint32_t table_ptrs[];
} __attribute__((packed));

struct ACPI_XSDT {
    struct ACPI_SDT_Descriptor_Header header;
    uint64_t table_ptrs[];
} __attribute__((packed));

void acpi_init(struct stivale_struct* stivale_struct);
bool acpi_validate_table(struct ACPI_SDT_Descriptor_Header* table_header); // Returns true if table is valid, else false
uint64_t acpi_find_table(char* table_signature);                    // Returns the address of the desired ACPI table based on its signature

#endif // RSDP_H