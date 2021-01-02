#ifndef KMALLOC_H
#define KMALLOC_H

#include <stdbool.h>
#include <stddef.h>
#include "../util/types.h"
#include "../driver/vga.h"

// PAGE TABLE LOCATION IN MEMORY
static uint64_t* PML4 = (uint64_t*)0x15000;
static uint64_t* PDPT = (uint64_t*)0x16000;
static uint64_t* PDT = (uint64_t*)0x18000;  // the PDT we use for the bootloader is at 0x17000 (we identity map the first 1 gib)

// MEMORY MAP LOCATION IN MEMORY - where we store our address range descriptors
static uint64_t* MEMORY_MAP_LOCATION = (uint64_t*)0x13000;

// PMM bitmap
static uint8_t* bitmap_start = (uint8_t*)0x14000;

extern void refresh_page_tables(void);
void zero_addr_range(uint64_t start, uint64_t bytes);
void map_page(uint64_t vaddr, uint64_t paddr, uint64_t flags);
void* kmalloc();    // BROKEN RIGHT NOW - WILL IMPLEMENT LATER

#endif // KMALLOC_H