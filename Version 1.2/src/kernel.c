#include "driver/vga/vga.h"
#include "driver/etc/io.h"

__attribute__((section(".text")))
int kmain() {
    vga_cls(VGA_COLOR_RED);
    for (size_t i = 0; i < 2000; i++) {
        vga_putc('A' + (i % 42), VGA_COLOR_WHITE << 4 | VGA_COLOR_BLUE);
    }
    vga_cls(VGA_COLOR_WHITE << 4 | VGA_COLOR_BLUE);
    char* str = "\n\tHello, world!";
    vga_print(str, 15, VGA_COLOR_WHITE << 4 | VGA_COLOR_BLUE);
    while (1 == 1) {

    }
    return 0;
}