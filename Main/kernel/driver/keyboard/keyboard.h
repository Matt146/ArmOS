#ifndef KEYBOARD_H
#define KEYBOARD_H

#include <stdint.h>
#include "../../lib/io.h"
#include "../../lib/util.h"
#include "../../int/lapic.h"

#define PS2_DATA 0x60       // R/W
#define PS2_STATUS 0x64     // R
#define PS2_COMMAND 0x64    // W

void ps2_keyboard_init();

void ps2_write(uint16_t data);
uint8_t ps2_read();

void ps2_incoming_interrupt_handler(void);
void ps2_irq1_handler(void);

#endif // KEYBOARD_H