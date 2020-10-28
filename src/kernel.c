__asm__("jmp kmain\n\t");

#include "util/vga.h"

/***********************************
 * IDT Code
***********************************/

struct IDTR {
        uint16_t size;          // size of the IDT (16 bits)
        uint64_t start;         // starting address of IDT (64 bits)
} __attribute__((packed));

struct IDT_Gate {
        uint16_t        offset_1;       // offset bits 0..15
        uint16_t        selector;       // the code segment selector in GDT or LDT
        uint16_t        options;        // configure interrupt stuff; look up what each bit does
        uint16_t        offset_2;       // offset bits 16..31
        uint32_t        offset_3;       // offset bits 32..63
        uint32_t        zero;           // reserved
} __attribute__((packed));

struct IDT_Gate IDT[256];

int kmain() {
        char x[] = {'H', 'e', 'l', 'l', 'o', '\n', '\0'};
        term_cls();
        while (true) {
                term_kprint(x, 0x1F);
        }
        __asm__("cli\nhlt\n");
}