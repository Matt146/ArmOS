#include "kmalloc.h"

void map_page(uint64_t vaddr, uint64_t paddr, uint64_t flags) {
    // Get page struct offsets associated with virtual addresss
    size_t pml4idx = (vaddr & ((uint64_t)0x1ff << 39)) >> 39;
    size_t pml3idx = (vaddr & ((uint64_t)0x1ff << 30)) >> 30;
    size_t pml2idx = (vaddr & ((uint64_t)0x1ff << 21)) >> 21;

    // Use offsets to obtain the addresses of the page structs associated with
    // said virtual addresses
    uint64_t* pml3 = (uint64_t*)(PML4[pml4idx] & ~0xfff);
    if (pml3 == NULL) {
        if ((uint64_t)PDPT == 0x16000) {
            PDPT = 0x600000;
        } else {
            PDPT += 0x1000;
        }
        PML4[pml4idx] = (uint64_t)PDPT | 0x3; // create a new page table structure
    }
    uint64_t* pml2 = (uint64_t*)(pml3[pml3idx] & ~0xfff);
    if (pml2 == NULL) {
        pml3[pml3idx] = (uint64_t)PDT | 0x3;
        PDT += 0x1000;
    }
    pml2[pml2idx] = (paddr | flags) | 0b10000000;
    refresh_page_tables();
}

void zero_addr_range(uint64_t start, uint64_t bytes) {
    for (start; start < start + bytes; start += 1) {
        unsigned char* x = (unsigned char*)start;
        *x = 0;
    }
}

void* kmalloc() {
    map_page(1677721600, 3355443200, 0x3);
    void* return_val = (void*)0x60000000;
    return return_val;
}