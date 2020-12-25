#include "kmalloc.h"

void identity_map_page(uint64_t* vaddr, uint64_t flags) {
    term_kprint("1.\n", 0x1F);
    // calculate page table entry pointer. (ie: pml4_ptr points to pml4's virtual address)
    uint64_t* pml4_ptr = ((uint64_t)vaddr & 0xFF8000000000) * 8 + PML4;
    uint64_t* pdpt_ptr = ((uint64_t)vaddr & 0x7FC0000000) * 8 + PDPT;
    uint64_t* pdt_ptr = ((uint64_t)vaddr & 0x3FE00000) * 8 + PDT;
    term_kprint("2.\n", 0x1F);

    // map pml4
    *pml4_ptr = (uint64_t)pdpt_ptr | flags;
    *pdpt_ptr = (uint64_t)pdt_ptr | flags;
    *pdt_ptr = ((uint64_t)vaddr | flags) | 0b1000000;
    term_kprint("3.\n", 0x1F);
}

void* kmalloc() {
    term_kprint("4.\n", 0x1F);
    identity_map_page((uint64_t*)cur_mem_addr, 0x3);
    term_kprint("5.\n", 0x1F);
    void* return_val = (void*)cur_mem_addr;
    term_kprint("6.\n", 0x1F);
    cur_mem_addr += 2097152;
    term_kprint("7.\n", 0x1F);
    refresh_page_tables();
    term_kprint("8.\n", 0x1F);
    return return_val;
}