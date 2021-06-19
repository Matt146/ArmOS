#ifndef MP_H
#define MP_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include "../mm/vmm.h"
#include "../mm/gdt.h"
#include "../int/idt.h"
#include "../lib/util.h"
#include "../int/lapic.h"
#include "../acpi/madt.h"

#define SCHED_PER_CORE_STACK_SIZE 5 // Amount of pages ther per-core stack should have
#define SCHED_SMP_TRAMPOLINE 0x70000

struct sched_per_core {
    uint64_t cr3;
    struct GDTR gdtr;
    struct IDTR idtr;
    uint64_t rsp;
    uint8_t lapic_id;
} __attribute__((packed));

extern sched_core_binary_begin;
extern sched_core_binary_end;

struct sched_per_core* mp_init_per_core(uint8_t lapic_id);
void mp_init();

#endif // MP_H