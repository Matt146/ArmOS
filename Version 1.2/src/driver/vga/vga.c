#include "vga.h"

void vga_cls(uint8_t color) {
    for (size_t i = 0; i < VGA_TERM_COLUMNS * VGA_TERM_ROWS; i++) {
        vga_putc(0x0, color);
    }
}

void vga_putc(char c, uint8_t color) {
    volatile uint16_t* vga = (uint16_t*)VGA_BUFFER_START;
    if (c != '\t' || c != '\n') {
        if (vga_csr_x >= VGA_TERM_COLUMNS) {
            vga_csr_x = 0;
            vga_csr_y += 1;
        }
        if (vga_csr_y >= VGA_TERM_ROWS) {
            vga_csr_y = 0;
            vga_csr_x = 0;
        }

        vga[vga_csr_y * VGA_TERM_COLUMNS + vga_csr_x] = ((uint16_t)color << 8) | c;
        vga_csr_x += 1;
    }
}

void vga_print(char* str, size_t len, uint8_t color) {
    for (size_t i = 0; i < len; i++) {
        if (str[i] == '\n') {
            vga_csr_y += 1;
        } else if (str[i] == '\t') {
            vga_csr_x += VGA_TERM_TAB_WIDTH;
        } else {
            vga_putc(str[i], color);
        }
    }
}