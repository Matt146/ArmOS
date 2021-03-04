#ifndef ISR_H
#define ISR_H

#include <stdint.h>

/*
 * IDT Gate "type" is defined as
 * P: Present (set to 0 for unused interrupts) [1 bit]
 * DPL: Descriptor Privelige Level [2 bits]
 * S: Storage Segment (set to 0 for interrupt and trap gates) [1 bit]
 * Type: 0xF = trap gate; 0xE = interrupt gate [3 bits]
 *      - Trap gate = doesn't clear IF flag
 *      - Interrupt gate = clears IF flag
 *      - Interrupt Gate Type: 0b10001111
 *      - Trap Gate Type: 
*/
typedef struct IDT_Gate {
    uint16_t offset0;   // offset low bits (0-15)
    uint16_t cs;        // cs selector (2 bytes) (expaneded into a qword when pushed onto interrupt stack)
    uint8_t zero0;       // zero (1 byte)
    uint8_t type;       // IDT type and attributes (1 byte)
    uint16_t offset1;   // offset middle bits (16..31)
    uint32_t offset2;   // offset high bits (32..63)
    uint32_t zero1;      // zero (4 bytes)
} __attribute__((packed));

typedef struct IDTR {
    uint16_t limit; // length of the IDT in bytes - 1
    uint64_t base;  // 64-bit linear address where the IDT starts
} __attribute__((packed));

static struct IDT_Gate idt[256];
static struct IDTR idtr;

void init_interrupts();
void set_idt_gate(struct IDT_Gate value, int index);
extern void load_idt(struct IDTR* idtr);

#endif // ISR_H