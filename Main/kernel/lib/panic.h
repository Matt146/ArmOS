#ifndef PANIC_H
#define PANIC_H

#include "../driver/serial/serial.h"

void panic(uint8_t *fb_addr, char* msg);

#endif // PANIC_H