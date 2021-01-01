#ifndef KMALLOC_H
#define KMALLOC_H

#include <stdbool.h>
#include <stddef.h>
#include "../util/types.h"
#include "../driver/vga.h"

typedef struct {
    uint64_t pml4idx;
    uint64_t pml3idx;
    uint64_t pml2idx;
    
    uint64_t* pml3;
    uint64_t* pml2;
} page_index_t;

// PAGE TABLE LOCATION IN MEMORY
static uint64_t* PML4 = (void*)0x15000;
static uint64_t* PDPT = (void*)0x16000;
static uint64_t* PDT = (void*)0x18000;  // the PDT we use for the bootloader is at 0x17000 (we identity map the first 1 gib)

extern void refresh_page_tables(void);
void zero_addr_range(uint64_t start, uint64_t bytes);
void setup_recursive_mapping();
void identity_map_page(uint64_t vaddr, uint64_t flags);   // map virtual address to physical address
void map_page(uint64_t vaddr, uint64_t paddr, uint64_t flags);
void* kmalloc();

#endif // KMALLOC_H