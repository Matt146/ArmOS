#include "vga.h"

void term_cls() {
    csr_x = 0;
    csr_y = 0;
    while (true) {
        if (csr_x > TERM_COLUMNS) {
            csr_x = 0;
            csr_y += 1;
        }
        if (csr_y > TERM_ROWS) {
            csr_y = 0;
            csr_x = 0;
            break;
        }
        
        volatile uint16_t* video = (volatile uint16_t*)0xb8000;
        video[csr_y * TERM_COLUMNS + csr_x] = (uint16_t)(' ' | (0x1F << 8));
        csr_x += 1;
    }
}

void term_kputc(char c, uint8_t term_color) {
    if (csr_x > TERM_COLUMNS) {
        csr_x = 0;
        csr_y += 1;
    }
    if (csr_y > TERM_ROWS) {
        csr_y = 0;
        csr_x = 0;
    }

    volatile uint16_t* video = (volatile uint16_t*)0xb8000;
    video[csr_y * TERM_COLUMNS + csr_x] = (uint16_t)(c | (0x1F << 8));
    csr_x += 1;
}

void term_kprint(char* str, uint8_t term_color) {
    for (int i = 0; str[i] != '\0'; i++) {
        if (str[i] == '\n') {
            csr_y += 1;
            csr_x = 0;
        } else if (str[i] == '\t') {
            csr_x += TERM_TAB_WIDTH;
        } else {
            term_kputc(str[i], term_color);
        }
    }
}
