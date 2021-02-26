#include "vga.h"

void vga_cls(uint8_t color) {

}

void vga_putc(char c, uint8_t color) {
    uint16_t* vga = (uint16_t*)VGA_BUFFER_START;
    if (vga_csr_x < VGA_TERM_ROWS) {
        vga[vga_csr_y * VGA_TERM_ROWS + vga_csr_x] = c | (color << 8);
        vga_csr_x += 1;
    } else {
        vga_csr_x = 0;
        if (vga_csr_y < VGA_TERM_COLUMNS) {
            vga_csr_y = 0;
        } else {
            vga_csr_y += 1;
        }
    }
}