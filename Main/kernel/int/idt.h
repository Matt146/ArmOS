#ifndef IDT_H
#define IDT_H

#include <stdint.h>
#include <stddef.h>

struct IDT_Gate {
    uint16_t offset0;   // offset bits 0..15
    uint16_t cs;  // a code segment selector in GDT or LDT
    uint8_t zero0;      // bits 0..2 holds Interrupt Stack Table offset, rest of bits zero.
    uint8_t type;  // type and attributes
    uint16_t offset1;   // offset bits 16..31
    uint32_t offset2;   // offset bits 32..63
    uint32_t zero1;     // reserved
} __attribute__((packed));

struct IDTR {
    uint16_t length;
    uint64_t base;
} __attribute__((packed));


struct IDT_Gate IDT[256];

void idt_init();
static inline void lidt(void* base, uint16_t size);
extern void isr_common_stub(void);

#endif // IDT_H