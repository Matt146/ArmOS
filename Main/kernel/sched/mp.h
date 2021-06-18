#ifndef MP_H
#define MP_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include "../mm/vmm.h"
#include "../lib/util.h"
#include "../int/lapic.h"
#include "../acpi/madt.h"

#define MP_REAL_MODE_STUB_LOCATION 0x70000

#define MP_RM_STUB_DP_CR3_OFFSET 0x10
#define MP_RM_STUB_DP_GDT_OFFSET (MP_RM_STUB_DP_CR3_OFFSET + 8)
#define MP_RM_STUB_DP_IDT_OFFSET (MP_RM_STUB_DP_GDT_OFFSET + 0x1000)

struct __sched_GeneralDescriptor {
    uint16_t length;
    uint64_t base;
} __attribute__((packed));

extern sched_core_binary_begin;
extern sched_core_binary_end;

void mp_init();

#endif // MP_H