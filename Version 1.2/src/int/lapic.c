#include "lapic.h"

void lapic_init_default() {
    LAPIC_BASE = 0xFEC00000;
}
void lapic_remap(uint32_t paddr) {
    wrmsr(0x1B, ((paddr) & 0xfffff0000) | 0x800);
    LAPIC_BASE = (uint64_t)((paddr) & 0xfffff0000);
}

void lapic_enable() {
    serial_puts("\n[+] LAPIC SPURIOUS INTERRUPT VEC VALUE: ");
    volatile uint32_t* spurious_interrupt_vector_register = (volatile uint32_t*)(LAPIC_BASE + 0x0F0);
    *spurious_interrupt_vector_register |= 0b100000000;
    serial_puts(unsigned_long_to_str(*spurious_interrupt_vector_register));
}

void lapic_disable() {
    volatile uint32_t* spurious_interrupt_vector_register = (volatile uint32_t*)(LAPIC_BASE + 0x0F0);
    *spurious_interrupt_vector_register = (*spurious_interrupt_vector_register | 0b100000000) ^ (1 << 8);
}

void lapic_lvt_init_timer() {
    volatile uint32_t* lapic_divide_register = (volatile uint32_t*)(LAPIC_BASE + 0x3E0);
    *lapic_divide_register = 0x1;
    serial_puts("\n[+] LAPIC LVT TIMER REGISTER VALUE: ");
    volatile uint32_t* lvt_timer_register = (volatile uint32_t*)(LAPIC_BASE + 0x320);
    *lvt_timer_register = 0x69 | 0x20000;
    serial_puts(unsigned_long_to_str(*lvt_timer_register));
}

void lapic_timer_set(uint32_t count) {
    volatile uint32_t* lapic_timer_initial_count = (volatile uint32_t*)(LAPIC_BASE + 0x380);
    *lapic_timer_initial_count = count;
}