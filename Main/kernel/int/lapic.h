#ifndef LAPIC_H
#define LAPIC_H

#include <stdint.h>
#include <stddef.h>
#include "../lib/io.h"
#include "../lib/util.h"

#define LAPIC_SPURIOUS_INTERRUPT_VECTOR 0xf0

#define LAPIC_LVT_CMCI 0x02F0
#define LAPIC_LVT_TIMER 0x0320
#define LAPIC_LVT_THERMAL_MONITOR 0x0330
#define LAPIC_LVT_PERFORMANCE_COUNTER 0x0340
#define LAPIC_LVT_LINT0 0x0350
#define LAPIC_LVT_LINT1 0x0360
#define LAPIC_LVT_ERROR 0x0370

#define LAPIC_INITIAL_COUNT 0x0380
#define LAPIC_CURRENT_COUNT 0x0390
#define LAPIC_DIVIDE_CONFIGURATION 0x03E0

#define LAPIC_TIMER_INTERRUPT_VECTOR 0x69

static uint64_t LAPIC_BASE;

void lapic_init();
void lapic_init_timer();
void lapic_set_timer(uint32_t value);

#endif //LAPIC_H