#ifndef PMM_H
#define PMM_H

#include <stddef.h>
#include <stdint.h>
#include "../../stivale.h"
#include "../lib/util.h"
#include "../lib/panic.h"

#define PMM_MMAP_MAX_ENTRIES 256

struct mmap_entry {
    uint64_t base;      // Physical address of base of the memory section
    uint64_t length;    // Length of the section
    uint32_t type;      // Type (described below)
    uint32_t unused;
} __attribute__((packed));

struct bitmap {
    uint64_t start;
    uint64_t end;
    uint8_t* bitmap;
    struct bitmap* next;
};

static struct bitmap* pmm_bitmap;
static struct mmap_entry pmm_mmap[PMM_MMAP_MAX_ENTRIES];

void pmm_init(struct stivale_struct *stivale_struct);
static uint64_t pmm_read_mmap(struct stivale_struct *stivale_struct);

#endif // PMM_H