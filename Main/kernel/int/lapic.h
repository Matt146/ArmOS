#ifndef LAPIC_H
#define LAPIC_H

#include <stdint.h>
#include <stddef.h>
#include "../lib/io.h"

static uint64_t LAPIC_BASE;

void lapic_init_default();
uint32_t lapic_get_base_addr();      // reads the LAPIC base address
void lapic_remap(uint32_t paddr);   // remap LAPIC registers to a base address

void lapic_enable();
void lapic_disable();

void lapic_lvt_init_timer();    // LAPIC LVT vector will be set to LAPIC_LVT_TIMER_INTERRUPT_VECTOR and timer will be in one-shot mode
void lapic_timer_set(uint32_t count);   // Number the LAPIC timer will count down from until it interrupts

#endif //LAPIC_H