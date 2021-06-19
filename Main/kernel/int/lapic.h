#ifndef LAPIC_H
#define LAPIC_H

#include <stdint.h>
#include <stddef.h>
#include "../lib/io.h"
#include "../lib/util.h"
#include "../driver/serial/serial.h"

// LAPIC Version Register
#define LAPIC_VERSION 0x020

// LAPIC Spurious Interrupt Vector Register
#define LAPIC_SPURIOUS_INTERRUPT_VECTOR 0xf0

// LAPIC LVT Registers
#define LAPIC_LVT_CMCI 0x02F0
#define LAPIC_LVT_TIMER 0x0320
#define LAPIC_LVT_THERMAL_MONITOR 0x0330
#define LAPIC_LVT_PERFORMANCE_COUNTER 0x0340
#define LAPIC_LVT_LINT0 0x0350
#define LAPIC_LVT_LINT1 0x0360
#define LAPIC_LVT_ERROR 0x0370

// LAPIC Timer -- Important Registers & Constants
#define LAPIC_INITIAL_COUNT 0x0380
#define LAPIC_CURRENT_COUNT 0x0390
#define LAPIC_DIVIDE_CONFIGURATION 0x03E0
#define LAPIC_TIMER_INTERRUPT_VECTOR 0x69

// LAPIC IPI Constants
#define LAPIC_ICR_HIGH 0x0310
#define LAPIC_ICR_LOW 0x0300    // Writing to this portion of the ICR causes an interrupt to be sent

static uint64_t LAPIC_BASE;

void lapic_init();
uint8_t lapic_get_current_id();

// LAPIC Timer facilities
void lapic_init_timer();
void lapic_set_timer(uint32_t value);

// LAPIC IPI facilities
void lapic_send_ipi(uint8_t lapic_id, uint32_t lower_icr_value);

#endif //LAPIC_H