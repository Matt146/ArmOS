#include "idt.h"

void set_idt_gate(struct IDT_Gate value, int index) {
    IDT[index] = value;
}

void idt_init() {
    struct IDT_Gate idt_default;
    uint64_t offset = (uint64_t)isr_common_stub;

    // Set the values of the default IDT gate
    idt_default.offset0 = (uint16_t)offset;
    idt_default.cs = 0x8;
    idt_default.zero0 = 0x0;
    idt_default.type = 0x8E;
    idt_default.offset1 = (uint16_t)(offset >> 16);
    idt_default.offset2 =  (uint32_t)(offset >> 32);
    idt_default.zero1 = 0x0;

    // Set the default IDT gate to all IDT entries
    for (size_t i = 0; i < 256; i++) {
        set_idt_gate(idt_default, i);
    }

    offset = (uint64_t)ps2_irq1_handler_stub;

    // Set the values of the default IDT gate
    idt_default.offset0 = (uint16_t)offset;
    idt_default.cs = 0x8;
    idt_default.zero0 = 0x0;
    idt_default.type = 0x8E;
    idt_default.offset1 = (uint16_t)(offset >> 16);
    idt_default.offset2 =  (uint32_t)(offset >> 32);
    idt_default.zero1 = 0x0;

    set_idt_gate(idt_default, 33);  // PS/2 Keyboard

    /*
    offset = (uint64_t)ps2_irq12_handler_stub;

    // Set the values of the default IDT gate
    idt_default.offset0 = (uint16_t)offset;
    idt_default.cs = 0x8;
    idt_default.zero0 = 0x0;
    idt_default.type = 0x8E;
    idt_default.offset1 = (uint16_t)(offset >> 16);
    idt_default.offset2 =  (uint32_t)(offset >> 32);
    idt_default.zero1 = 0x0;

    set_idt_gate(idt_default, 44);  // PS/2 MOUSE
    */

    lidt(&IDT, 16*256 - 1);
}

static inline void lidt(void* base, uint16_t size) {
    struct {
        uint16_t length;
        void*    base;
    } __attribute__((packed)) IDTR = { size, base };

    asm ( "lidt %0" : : "m"(IDTR) );  // let the compiler choose an addressing mode
}