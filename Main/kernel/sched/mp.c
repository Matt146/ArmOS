#include "mp.h"

struct sched_per_core* mp_init_per_core(uint8_t lapic_id) {
    uint64_t per_core_paddr = pmm_alloc(1);
    struct sched_per_core* per_core = (struct sched_per_core*)per_core_paddr;

    // Per core paging
    per_core->cr3 = vmm_get_cr3();

    // Per core GDT
    asm volatile ("sgdt %0" :: "m"(per_core->gdtr));
    per_core->gdtr.base -= KERNEL_HIGH_VMA;

    // Per core IDT
    asm volatile ("sidt %0" :: "m"(per_core->idtr));
    per_core->idtr.base -= KERNEL_HIGH_VMA;

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
            // Prepare per-core area
            struct sched_per_core* per_core_loc = mp_init_per_core(acpi_processors[i]);
            memcpy((struct sched_per_core*)SCHED_PER_CORE, per_core_loc, sizeof(struct sched_per_core));

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