#ifndef MP_H
#define MP_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include "../lib/util.h"
#include "../int/lapic.h"
#include "../acpi/madt.h"

#define MP_REAL_MODE_STUB_LOCATION 0x70000

extern sched_core_binary_begin;
extern sched_core_binary_end;

void mp_init();

#endif // MP_H