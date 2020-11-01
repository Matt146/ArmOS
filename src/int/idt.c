#include "idt.h"

// assign_isr - assigns an ISR to the correct IDT gate
void assign_isr(uint8_t vector_num,  uint64_t isr_offset, uint16_t cs_selector, uint16_t options) {
    // create the offset masks
    uint64_t offset_1_mask = create_bitmask(0, 15);
    uint64_t offset_2_mask = create_bitmask(16, 31);
    uint64_t offset_3_mask = create_bitmask(32, 63);

    // assign to IDT
    IDT[vector_num].offset_1 = isr_offset & offset_1_mask;
    IDT[vector_num].selector = cs_selector;
    IDT[vector_num].options = options;
    IDT[vector_num].offset_2 = isr_offset & offset_2_mask;
    IDT[vector_num].offset_3 = isr_offset & offset_3_mask;
    IDT[vector_num].zero = 0; 
}

// load_idt - loads the IDT using the idtr
void load_idt(struct IDTR idtr) {
    idtr.size = sizeof(IDT);
    idtr.start = &IDT;
    __asm__ __volatile__("lidt %0" : : "m"(idtr));
}

// init_idt - sets up IDT
void init_idt() {
    for (int i = 0; i < 256; i++) {
        uint16_t cs_value = 8;
        assign_isr(i, &isr_common_stub, cs_value, 0x1);
    }
}