#ifndef MADT_H
#define MADT_H

#include "rsdp.h"

#define ACPI_MAX_CORES 256

// Contains the LAPIC ID's of a bunch of processors
static uint8_t acpi_processors[ACPI_MAX_CORES];

struct MADT_SpecialHeader {
    uint32_t lapic_addr;
    uint32_t flags;
} __attribute__((packed));

struct MADT {
    struct ACPI_SDT_Descriptor_Header header;
    struct MADT_SpecialHeader special;
    uint32_t zero;
} __attribute__((packed));

// MADT Entry Type 0 corresponds to a processor
// local APIC
struct MADT_EntryType0 {
    uint16_t zero;
    uint8_t acpi_id;
    uint8_t lapic_id;
    uint32_t flags;
} __attribute__((packed));

// MADT Entry Type 1 corresponds to an IOAPIC
struct MADT_EntryType1 {
    uint16_t zero;
    uint8_t ioapic_id;
    uint8_t reserved;
    uint32_t ioapic_addr;
    uint32_t global_system_interrupt_base;
} __attribute__((packed));

// MADT Entry Type 2: IOAPIC Interrupt Source Override
struct MADT_EntryType2 {
    uint16_t zero;
    uint8_t bus_source;
    uint8_t irq_source;
    uint32_t global_system_interrupt;
    uint16_t flags;
} __attribute__((packed));

// MADT Entry Type 3: IOAPIC NMI Source
struct MADT_EntryType3 {
    uint16_t zero;
    uint8_t nmi_source;
    uint8_t reserved;
    uint16_t flags;
    uint32_t global_system_interrupt;
} __attribute__((packed));

// MADT Entry Type 4: LAPIC NMI Interrupts
struct MADT_EntryType4 {
    uint16_t zero;
    uint8_t acpi_processor_id;  // 0xFF means all processors
    uint16_t flags;
    uint8_t lint;
} __attribute__((packed));

// MADT Entry Type 5: LAPIC Address Override
struct MADT_EntryType5 {
    uint32_t zero;
    uint64_t lapic_addr
} __attribute__((packed));

// MADT Entry Type 9: Processor Local x2APIC
struct MADT_EntryType9 {
    uint32_t reserved;
    uint32_t processor_local_x2apic_id;
    uint32_t flags;
    uint32_t acpi_id;
} __attribute__((packed));

void acpi_madt_detect_cores(uint64_t madt_addr);

#endif // MADT_H