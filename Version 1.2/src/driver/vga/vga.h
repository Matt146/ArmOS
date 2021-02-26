#ifndef VGA_H
#define VGA_H

#include <stddef.h>
#include <stdint.h>

// Terminal Constants
#define VGA_TERM_ROWS 25
#define VGA_TERM_COLUMNS 80
#define VGA_TERM_TAB_WIDTH 8
#define VGA_BUFFER_START 0xB8000

// Stores the position of the cursor
static size_t vga_csr_x = 0;
static size_t vga_csr_y = 0;

// Color definitions
static const uint8_t VGA_COLOR_BLACK       = 0x00;
static const uint8_t VGA_COLOR_BLUE        = 0x01;
static const uint8_t VGA_COLOR_GREEN       = 0x02;
static const uint8_t VGA_COLOR_CYAN        = 0x03;
static const uint8_t VGA_COLOR_RED         = 0x04;
static const uint8_t VGA_COLOR_MAGENTA     = 0x05;
static const uint8_t VGA_COLOR_BROWN       = 0x06;
static const uint8_t VGA_COLOR_LIGHTGRAY   = 0x07; 
static const uint8_t VGA_COLOR_DARKGRAY    = 0x08;
static const uint8_t VGA_COLOR_LIGHTBLUE   = 0x09;
static const uint8_t VGA_COLOR_LIGHTGREEN  = 0x0A;
static const uint8_t VGA_COLOR_LIGHTCYAN   = 0x0B;
static const uint8_t VGA_COLOR_LIGHTRED    = 0x0C;
static const uint8_t VGA_COLOR_LIGHTMAGENTA= 0x0D;
static const uint8_t VGA_COLOR_YELLOW      = 0x0E;
static const uint8_t VGA_COLOR_WHITE       = 0x0F;

// Functions here
void vga_cls(uint8_t color);
void vga_putc(char c, uint8_t color);
void vga_print(char* str, size_t len, uint8_t color);

#endif // VGA_H