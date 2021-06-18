#include "mp.h"

void mp_init() {
    // Memcpy the real mode stub to 0x70000 (where it needs to be in memory)
    uint8_t* start = &sched_core_binary_begin;
    uint8_t* end = &sched_core_binary_end;
    memsetb(MP_REAL_MODE_STUB_LOCATION, 0, end-start);
    memcpy(MP_REAL_MODE_STUB_LOCATION, start, end-start);

    struct __sched_GeneralDescriptor gdtr = {4096 - 1, (MP_REAL_MODE_STUB_LOCATION + MP_RM_STUB_DP_GDT_OFFSET)};
    asm volatile ("sgdt %0" : : "m"(gdtr));

    struct __sched_GeneralDescriptor idtr = {4096 - 1, (MP_REAL_MODE_STUB_LOCATION + MP_RM_STUB_DP_IDT_OFFSET)};
    asm volatile ("sidt %0" : : "m"(idtr));

    uint64_t* cr3_writer = (uint64_t*)(MP_REAL_MODE_STUB_LOCATION + MP_RM_STUB_DP_CR3_OFFSET);
    *cr3_writer = vmm_get_cr3();

    // Send INIT IPI's and SIPI's to core
    for (size_t i = 0; i < acpi_detected_processors_count; i++) {
        if (acpi_processors[i] != lapic_get_current_id()) {
            // Send INIT
            serial_puts("\n[SCHED] Sending INIT to core ");
            serial_puts(unsigned_long_to_str(acpi_processors[i]));
            lapic_send_ipi(acpi_processors[i], 0b101 << 8);

            // Send SIPI
            serial_puts("\n[SCHED] Sending SIPI to core ");
            serial_puts(unsigned_long_to_str(acpi_processors[i]));
            lapic_send_ipi(acpi_processors[i], (0b110 << 8) | (0x70));
        }
    }
}