#include "lapic.h"

void lapic_init() {
    // Establish the LAPIC base from the MSR
    uint64_t lapic_base_addr = rdmsr(0x1B);
    LAPIC_BASE = ((lapic_base_addr >> 12) & 0xffffff) << 12;
    serial_puts("\n[LAPIC] LAPIC IA32_APIC_BASE MSR value: ");
    serial_puts(unsigned_long_to_str((uint64_t)LAPIC_BASE));

    // Enable the LAPIC
    volatile uint32_t* lapic_spurious_interrupt_vec_ptr = (volatile uint32_t*)(LAPIC_BASE + LAPIC_SPURIOUS_INTERRUPT_VECTOR);
    serial_puts("\n[LAPIC] LAPIC Spurious Interrupt Vector Ptr: ");
    serial_puts(unsigned_long_to_str((uint64_t)lapic_spurious_interrupt_vec_ptr));
    *lapic_spurious_interrupt_vec_ptr |= 0x1ff;
}

uint8_t lapic_get_current_id() {
    volatile uint32_t* lapic_version = (volatile uint32_t*)(LAPIC_BASE + LAPIC_VERSION);
    return (uint8_t)((*lapic_version >> 24) & 0x7);
}

void lapic_init_timer() {
    // Set timer to periodic mode and the interrupt vector to 0x69
    volatile uint32_t* lapic_timer = (volatile uint32_t*)(LAPIC_BASE + LAPIC_LVT_TIMER);
    *lapic_timer |= (LAPIC_TIMER_INTERRUPT_VECTOR | (0b01 << 17));
}

void lapic_set_timer(uint32_t value) {
    volatile uint32_t* lapic_initial_count = (volatile uint32_t*)(LAPIC_BASE + LAPIC_INITIAL_COUNT);
    *lapic_initial_count = value;
}

void lapic_send_ipi(uint8_t lapic_id, uint32_t lower_icr_value) {
    volatile uint32_t* lapic_icr_upper_dword = (volatile uint32_t*)(LAPIC_BASE + LAPIC_ICR_HIGH);
    volatile uint32_t* lapic_icr_lower_dword = (volatile uint32_t*)(LAPIC_BASE + LAPIC_ICR_LOW);

    // Write the destination field to the upper dword
    *lapic_icr_upper_dword = (uint32_t)lapic_id << (32 - 8);

    // Print some debug info
    serial_puts("\n[LAPIC] Sending IPI...");
    serial_puts("\n - ICR Upper value: ");
    serial_puts(unsigned_long_to_str((uint64_t)*lapic_icr_upper_dword));
    serial_puts("\n - ICR Lower value: ");
    serial_puts(unsigned_long_to_str((uint64_t)lower_icr_value));

    // Wait for all previous IPI's to send by checking the status bit
    // When it's done, send an IPI by writing to the lower DWORD
    while ((*lapic_icr_lower_dword & (1 << 11))) {

    }
    *lapic_icr_lower_dword = lower_icr_value;
}