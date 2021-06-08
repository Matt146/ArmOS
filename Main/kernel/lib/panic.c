#include "panic.h"

void panic(uint8_t *fb_addr, char* msg) {
    serial_puts("\n[PANIC] ");
    serial_puts(msg);
    // TEMPORARY
    for (size_t i = 0; i < 128*30; i++) {
        fb_addr[i] = 0xff;
    }

    // We're done, just hang...
    for (;;) {
        asm ("hlt");
    }
}