#ifndef SERIAL_H
#define SERIAL_H

#include <stdbool.h>
#include <stdint.h>
#include "../etc/io.h"

// COM1 and COM2 are the most standardized ports, so we don't have to get data from BIOS Data Area
// or use manual probing technqiues. Also, manual probing is a better way to go about things according
// to the osdev wiki. COM1 + n will give you the IO address of some data register mapped to that port.
#define COM1 0x3F8  // IO port for the COM1 interface
#define DIVISOR_LOW 0x2     // Set the divisor to 2 because that'll give me 57600 baud
#define DIVISOR_HIGH 0x00

void init_serial();
bool serial_transmission_is_ready(); // returns true if you can write to data port, returns false if you have to wait
void serial_putc(char c);


#endif // SERIAL_H
