#ifndef DEBUG_H
#define DEBUG_H

#include <stdarg.h>
#include "../driver/serial/serial.h"

void print_format(void (*put)(char), const char *str, va_list arg);
void print(const char *str, ...);

#endif
