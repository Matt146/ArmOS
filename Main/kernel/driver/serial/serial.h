#ifndef SERIAL_H
#define SERIAL_H

#include <stdint.h>
#include <stdbool.h>
#include "../../lib/io.h"
#include "../../sched/mutex.h"

#define COM1 0x3F8
#define DIVISOR_LOW 0x2     // Set the divisor to 2 because that'll give me 57600 baud
#define DIVISOR_HIGH 0x00

void serial_init();
bool serial_transmission_is_ready();
void serial_putc(char c);
void serial_puts(char* str);

#endif // SERIAL_H
