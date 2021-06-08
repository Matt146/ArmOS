#include "pmm.h"

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

void read_bios_memory_map() {
    uint32_t* cur_memory_map_location = (uint32_t*)MEMORY_MAP_LOCATION;
    while (true) {
        if ((*cur_memory_map_location) == 0xdeadbeef) {
            term_kprint("[KERNEL] Read memory map!", 0x1F);
            break;
        }
        if (cur_address_range_descriptor > 99) {
            term_kprint("[ERROR] Went over kernel memory map region!\n", 0x1F);
            break;
        }

        // type
        uint32_t type = *cur_memory_map_location;
        cur_memory_map_location += 4;

        // length
        uint32_t limit_high = *cur_memory_map_location;
        cur_memory_map_location += 4;
        uint32_t limit_low = *cur_memory_map_location;
        cur_memory_map_location += 4;

        // base
        uint32_t base_high = *cur_memory_map_location;
        cur_memory_map_location += 4;
        uint32_t base_low = *cur_memory_map_location;
        cur_memory_map_location += 4;

        cur_memory_map_location += 4;

        // fill out the struct of address base descriptors
        address_range_descriptors[cur_address_range_descriptor].type = type;
        address_range_descriptors[cur_address_range_descriptor].base = ((uint64_t)((uint64_t)base_high << 32)) | base_low;
        address_range_descriptors[cur_address_range_descriptor].length = ((uint64_t)((uint64_t)limit_high << 32)) | limit_low;
        cur_address_range_descriptor += 1;
    }

/*
    term_kprint("[KERNEL] MEMORY MAP:\n", 0x1F);
    for (size_t i = 0; i < 2; i++) {
        term_kprint(long_to_str(address_range_descriptors[i].base), 0x1F);
        term_kprint("-", 0x1F);
        term_kprint(long_to_str(address_range_descriptors[i].length), 0x1F);
        term_kprint(" ", 0x1F);
        term_kprint(long_to_str(address_range_descriptors[i].type), 0x1F);
        term_kprint("\n", 0x1F);
    }
*/
}

void* kmalloc() {
    //map_page(1677721600, 3355443200, 0x3);
    void* return_val = (void*)0x60000000;
    return return_val;
}