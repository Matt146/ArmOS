#ifndef KMALLOC_H
#define KMALLOC_H

#include <stdbool.h>
#include <stddef.h>
#include "../util/types.h"
#include "../driver/vga.h"

// PAGE TABLE LOCATION IN MEMORY
static void* PML4 = (void*)0x15000;
static void* PDPT = (void*)0x16000;
static void* PDT = (void*)0x18000;  // the PDT we use for the bootloader is at 0x17000 (we identity map the first 1 gib)

static uint64_t cur_mem_addr = 0x40000000;
static uint64_t cur_pml4_entry = 0;
static uint64_t cur_pdpt_entry = 1;
static uint64_t cur_pdt_entry = 0;

extern void refresh_page_tables(void);
void zero_addr_range(uint64_t start, uint64_t bytes);
void setup_recursive_mapping();
void identity_map_page(uint64_t vaddr, uint64_t flags);   // map virtual address to physical address
void* kmalloc();

#endif // KMALLOC_H