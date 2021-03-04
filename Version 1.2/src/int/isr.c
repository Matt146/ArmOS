#include "isr.h"

void init_interrupts() {
    idtr.base = (uint64_t)&idt;
    idtr.limit = 4095;
    load_idt(&idtr);
}

void set_idt_gate(struct IDT_Gate value, int index) {
    if (index >= 0 && index <= 255) {
        idt[index] = value;
    }
}