#include "kmalloc.h"

void identity_map_page(uint64_t vaddr, uint64_t flags) {
    term_kprint("1.\n", 0x1F);
    uint64_t* pml4_ptr = PML4;
    uint64_t* pdpt_ptr = PDPT;
    uint64_t* pdt_ptr = PDT;
    term_kprint("2. \n", 0x1F);

    // map pages
    *(pml4_ptr + cur_pml4_entry * 8) = (uint64_t)pdpt_ptr | 0x3;
    *(pdpt_ptr + cur_pdpt_entry * 8) = (uint64_t)pdt_ptr | 0x3;
    *(pdt_ptr + cur_pdt_entry * 8) = (vaddr | flags) | 0b10000011;
    cur_pdt_entry += 1;
    term_kprint("3. \n", 0x1F);

    if (cur_pdt_entry >= 512) {
        cur_pdt_entry = 0;
        PDT += 0x1000;
        cur_pdpt_entry += 1;
    }

    if (cur_pdpt_entry >= 512) {
        cur_pdpt_entry = 0;
        if (PDPT == 0x16000) {
            PDPT += 0x600000;
        } else {
            PDPT += 0x1000;
        }
        cur_pml4_entry += 1;
    }

    if (cur_pml4_entry >= 512) {
        term_kprint("Ran outta memory, faggot.", 0x1F);
    }
}

void* kmalloc() {
    identity_map_page(cur_mem_addr, 0x3);
    void* return_val = (void*)cur_mem_addr;
    cur_mem_addr += 0x200000;
    refresh_page_tables();
    return return_val;
}