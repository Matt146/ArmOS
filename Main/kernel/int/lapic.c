#include "lapic.h"

void lapic_init() {
    // Establish the LAPIC base from the MSR
    uint64_t lapic_base_addr = rdmsr(0x1B);
    LAPIC_BASE = ((lapic_base_addr >> 12) & 0xffffff) << 12;
    serial_puts("\n[LAPIC] LAPIC IA32_APIC_BASE MSR value: ");
    serial_puts(unsigned_long_to_str(LAPIC_BASE));

    // Enable the LAPIC
    volatile uint32_t* lapic_spurious_interrupt_vec_ptr = (volatile uint32_t*)(LAPIC_BASE + LAPIC_SPURIOUS_INTERRUPT_VECTOR);
    serial_puts("\n[LAPIC] LAPIC Spurious Interrupt Vector Ptr: ");
    serial_puts(unsigned_long_to_str(lapic_spurious_interrupt_vec_ptr));
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

void lapic_send_ipi(struct ICR* icr) {
    volatile uint32_t* lapic_icr_upper_dword = (volatile uint32_t*)(LAPIC_BASE + LAPIC_ICR_HIGH);
    volatile uint32_t* lapic_icr_lower_dword = (volatile uint32_t)(LAPIC_BASE + LAPIC_ICR_LOW);

    // Write the destination field to the upper dword
    *lapic_icr_upper_dword = (icr->destination << 56);

    // Prepare the value for the lower dword, as writing to it will cause the IPI to be sent
    uint32_t lapic_icr_lower_dword_value = 0;
    lapic_icr_lower_dword_value = icr->vector;                      // vector
    lapic_icr_lower_dword_value |= (icr->send_options & 0x7) << 8;  // delivery mode
    lapic_icr_lower_dword_value |= (icr->send_options & (1 << 6)) << 8;  // level
    lapic_icr_lower_dword_value |= (icr->send_options & (1 << 7)) << 8;  // trigger mode
    lapic_icr_lower_dword_value |= (icr->send_options & (0b11 << 10)) << 8; // destination shorthand

    serial_puts("\n[LAPIC] Sending IPI...");
    serial_puts("\n - ICR Upper value: ");
    serial_puts(unsigned_long_to_str(*lapic_icr_upper_dword));
    serial_puts("\n - ICR Lower value: ");
    serial_puts(unsigned_long_to_str(lapic_icr_lower_dword_value));


    while ((*lapic_icr_lower_dword & (1 << 11))) {

    }
    *lapic_icr_lower_dword = lapic_icr_lower_dword_value;
}