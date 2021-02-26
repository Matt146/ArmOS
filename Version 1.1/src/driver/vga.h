#ifndef UTILS_H
#define UTILS_H

#include "../util/types.h"

/*****************************************
 * Printing functions
*****************************************/


#define TERM_ROWS 25
#define TERM_COLUMNS 80
#define TERM_TAB_WIDTH 8

static size_t csr_x = 0;
static size_t csr_y = 0;

void term_cls();                                     // clears the screen and sets the background
void term_kputc(char c, uint8_t term_color);         // prints a single letter on the screen
void term_kprint(char* str, uint8_t term_color);     // prints a string; NOTE: assumes null termination

#endif // UTILS_H