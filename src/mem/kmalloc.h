#ifndef KMALLOC_H
#define KMALLOC_H

#include <stdbool.h>
#include <stddef.h>
#include "../util/types.h"
#include "../driver/vga.h"

// PAGE TABLE LOCATION IN MEMORY
static void* PML4 = (void*)0x15000;
static void* PDPT = (void*)0x16000;
static void* PDT = (void*)0x17000;

// WHAT MEMORY ADDRESS WE'RE CURRENTLY ON
static uint64_t cur_mem_addr = 0x124F80;
// You need to use 2mb physical alignment for the final level
// You need to use 2mb physical alignment for the final level
// You need to use 2mb physical alignment for the final level
// WHY IT DOESN'T WORK ^^^ WE GET STUCK AT 0x92BB - fuck
extern void refresh_page_tables(void);
void setup_recursive_mapping();
void identity_map_page(uint64_t* vaddr, uint64_t flags);   // map virtual address to physical address
void* kmalloc();

#endif // KMALLOC_H