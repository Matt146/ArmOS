#ifndef IO_H_FILE
#define IO_H

#include "types.h"

extern void outb(uint16_t port, uint8_t data);
extern uint8_t inb(uint16_t port);

#endif // IO_H