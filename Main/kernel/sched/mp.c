#include "mp.h"

struct sched_per_core* mp_init_per_core(uint8_t lapic_id) {
    uint64_t per_core_paddr = pmm_alloc(1);
    struct sched_per_core* per_core = (struct sched_per_core*)per_core_paddr;

    // Per core paging
    per_core->cr3 = vmm_get_cr3();

    // Per core GDT
    per_core->gdtr.length = 39;
    per_core->gdtr.base = pmm_alloc(1);

    // Per core IDT
    per_core->idtr.length = 4095;
    per_core->idtr.base = pmm_alloc(1);

    // Per core rsp
    per_core->rsp = pmm_alloc(SCHED_PER_CORE_STACK_SIZE) + PMM_PAGE_SIZE * SCHED_PER_CORE_STACK_SIZE;

    // Per core LAPIC ID
    per_core->lapic_id = lapic_id;

    return per_core;
}

void mp_init() {
    // Memcpy the SMP trampoline code to 0x70000
    serial_puts("\n[SCHED] Memcpy'ing SMP trampoline code to 0x70000");
    serial_puts("\n - Sched core binary begin: ");
    serial_puts(unsigned_long_to_str(&sched_core_binary_begin));
    serial_puts("\n - Sched core binary end: ");
    serial_puts(unsigned_long_to_str(&sched_core_binary_end));
    serial_puts("\n - Sched core binary length: ");
    serial_puts(unsigned_long_to_str((uint64_t)&sched_core_binary_end - (uint64_t)&sched_core_binary_begin));
    memcpy((uint8_t*)SCHED_SMP_TRAMPOLINE, &sched_core_binary_begin, (uint64_t)&sched_core_binary_end - (uint64_t)&sched_core_binary_begin);

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