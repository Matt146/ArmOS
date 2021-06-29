#ifndef MOUSE_H
#define MOUSE_H

#include <stdint.h>
#include "../keyboard/keyboard.h"
#include "../serial/serial.h"
#include "../../int/ioapic.h"
#include "../../int/lapic.h"

struct PS2_Mouse {
    int32_t mouse_x;
    int32_t mouse_y;
} __attribute__((packed));

#endif // MOUSE_H