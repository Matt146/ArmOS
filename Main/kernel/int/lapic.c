#include "lapic.h"

void lapic_init() {
    // Establish the LAPIC base from the MSR
    uint64_t lapic_base_addr = rdmsr(0x1B);
    LAPIC_BASE = ((lapic_base_addr >> 12) & 0xffffff) << 12;
    serial_puts("\n[LAPIC] LAPIC IA32_APIC_BASE MSR value: ");
    serial_puts(unsigned_long_to_str(LAPIC_BASE));

    // Enable the LAPIC
    uint32_t* lapic_spurious_interrupt_vec_ptr = (uint32_t*)(LAPIC_BASE + LAPIC_SPURIOUS_INTERRUPT_VECTOR);
    serial_puts("\n[LAPIC] LAPIC Spurious Interrupt Vector Ptr: ");
    serial_puts(unsigned_long_to_str(lapic_spurious_interrupt_vec_ptr));
    *lapic_spurious_interrupt_vec_ptr |= 0x1ff;
}

void lapic_init_timer() {
    // Set timer to periodic mode and the interrupt vector to 0x69
    uint32_t* lapic_timer = (uint32_t*)(LAPIC_BASE + LAPIC_LVT_TIMER);
    *lapic_timer |= (LAPIC_TIMER_INTERRUPT_VECTOR | (0b01 << 17));
}

void lapic_set_timer(uint32_t value) {
    uint32_t* lapic_initial_count = (uint32_t*)(LAPIC_BASE + LAPIC_INITIAL_COUNT);
    *lapic_initial_count = value;
}