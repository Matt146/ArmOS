#include "vga.h"

void vga_cls(uint8_t color) {
    for (size_t i = 0; i < VGA_TERM_COLUMNS * VGA_TERM_ROWS; i++) {
        vga_putc(0x0, color);
    }
    vga_csr_x = 0;
    vga_csr_y = 0;
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
            for (size_t i = vga_csr_x; i < VGA_TERM_ROWS; i++) {
                vga_putc(0x0, color);
            }
            vga_csr_y += 1;
            vga_csr_x = 0;
        } else if (str[i] == '\t') {
            vga_csr_x += VGA_TERM_TAB_WIDTH;
        } else {
            vga_putc(str[i], color);
        }
    }
}

void vga_puts(char* str, uint8_t color) {
    size_t count = 0;
    // vga_csr_y += 1;
    // vga_csr_x = 0;
    while (1 == 1) {
        if (str[count] == '\0') {
            break;
        }
        if (str[count] == '\n') {
            for (size_t i = vga_csr_x; i < VGA_TERM_ROWS; i++) {
                vga_putc(0x0, color);
            }
            vga_csr_y += 1;
            vga_csr_x = 0;
        } else if (str[count] == '\t') {
            vga_csr_x += VGA_TERM_TAB_WIDTH;
        } else {
            vga_putc(str[count], color);
        }
        count += 1;
    }
}

char* long_to_str(long zahl) {
   static char text[20];   //Make me static, otherwise it's on the stack and will screw up soon, if it's static, it's allocated always, but is not safe for multi-tasking/threading.
   int loc=19;
   text[19] = 0; //NULL terminate the string
   do  //While we have something left, lets add a character to the string
   {
       --loc;
       text[loc] = (zahl%10)+'0';
       zahl/=10;
   } while (zahl);
   return &text[loc];  //Start from where loc left off
}

char* ___text_buff_unsigned_long_to_str = 0x10;
char* unsigned_long_to_str(unsigned long zahl) {
    for (size_t i = 0; i < 20; i++) {
        ___text_buff_unsigned_long_to_str[i] = '\0';
    }
    char* text =  ___text_buff_unsigned_long_to_str;  //Make me static, otherwise it's on the stack and will screw up soon, if it's static, it's allocated always, but is not safe for multi-tasking/threading.
   int loc=19;
   text[19] = 0; //NULL terminate the string
   do  //While we have something left, lets add a character to the string
   {
       --loc;
       text[loc] = (zahl%10)+'0';
       zahl/=10;
   } while (zahl);
   return &text[loc];  //Start from where loc left off
}
/*
char* unsigned_long_to_str(unsigned long zahl) {
    char digits[] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'};
    for (size_t i = 0; i < 18; i++) {
        ___text_buff_unsigned_long_to_str[i] = '\0';
    }
    char* text =  ___text_buff_unsigned_long_to_str;  //Make me static, otherwise it's on the stack and will screw up soon, if it's static, it's allocated always, but is not safe for multi-tasking/threading.
    int loc=16;
    text[16] = 0; //NULL terminate the string
    do  //While we have something left, lets add a character to the string
    {
        *--text = digits[zahl % 16];
        zahl/=16;
    } while (zahl);
    text[loc-1] = 'x';
    text[loc-2] = '0';
    return &text[loc-2];  //Start from where loc left off
}
*/