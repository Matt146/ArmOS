#include "idt.h"

// assign_isr - assigns an ISR to the correct IDT gate
void assign_isr(uint8_t vector_num, uint64_t isr_offset, uint16_t cs_selector, uint8_t ist, uint8_t attributes) {
    // assign to IDT
    IDT[vector_num].offset_1 = (uint16_t)((uint64_t)isr_offset & 0xFFFF);
    IDT[vector_num].selector = cs_selector;
    IDT[vector_num].ist = ist;
    IDT[vector_num].attributes = attributes;
    IDT[vector_num].offset_2 = (uint16_t)(((uint64_t)isr_offset >> 16) & 0xFFFF);
    IDT[vector_num].offset_3 = (uint32_t)(((uint64_t)isr_offset >> 32) & 0xFFFFFFFF);
    IDT[vector_num].zero = 0; 
}

// load_idt - loads the IDT base and limit into the IDTR register using the lidt instruction
void load_idt() {
    idtr.size = 4096;
    idtr.start = (uint64_t)&IDT;
    __asm__ __volatile__("lidt %0" : : "m"(idtr));
}

// init_idt - sets up IDT
void init_idt() {
    for (int i = 0; i < 256; i++) {
        uint16_t cs_value = 0x8;
        assign_isr(i, (uint64_t)&isr_common_stub, cs_value, 0x0, 0x8e);
    }
    load_idt();
}