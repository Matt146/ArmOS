#include "driver/vga/vga.h"
#include "driver/etc/io.h"

__attribute__((section(".text")))
int kmain() {
    DEBUG();
    for (size_t i = 0; i < 1000; i++) {
        vga_putc('A', (VGA_COLOR_RED << 4) | VGA_COLOR_BLUE);
    }
    return 0;
}