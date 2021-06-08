#ifndef E9_H
#define E9_H

#include "../etc/io.h"
#include <stdint.h>
#include <stdbool.h>

static bool e9_enabled = false;

void init_e9();
void e9_putc(char c);
void e9_puts(char* str);

#endif
