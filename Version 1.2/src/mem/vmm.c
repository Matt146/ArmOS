#include "vmm.h"

void vmm_init() {
    size_t start = VMM_PAGE_TABLES_START;
    for (start; start < VMM_PAGE_TABLES_END; start += 1) {
        unsigned char* x = (unsigned char*)start;
        *x = 0;
    }
    for (size_t i = 0; i < 1073741824; i+=0x200000) {
        vmm_map_vaddr_to_paddr(i, i, 0x3);
    }
    set_cr3(VMM_CR3);
}

/*
uint64_t vmm_map_vaddr_to_paddr(uint64_t paddr, uint64_t vaddr, uint8_t flags) {
    // Get page struct offsets associated with virtual addresss
    size_t pml4idx = (vaddr & ((uint64_t)0x1ff << 39)) >> 39;
    size_t pml3idx = (vaddr & ((uint64_t)0x1ff << 30)) >> 30;
    size_t pml2idx = (vaddr & ((uint64_t)0x1ff << 21)) >> 21;

    if (pml4idx == 0) {
        PML4[pml4idx] = VMM_PML3_INITIAL | 0x3;
    } else {
        uint64_t pml4_entry_value = VMM_PML3_START + 0x200 * pml4idx;
        PML4[pml4idx] = pml4_entry_value | 0x3;
    }

    // PDPT --> PDT
    if (pml3idx == 0) {
        PDPT[pml3idx] = VMM_PML2_INITIAL | 0x3;
    } else {
        uint64_t pdpt_entry_value = VMM_PML2_START + 0x200 * pml3idx + 0x200 * 0x200 * pml4idx;
        *((uint64_t*)(VMM_PML3_START + 0x200 * pml4idx + pml3idx)) =  pdpt_entry_value | 0x3;
    }
    serial_puts("\n");

    // PDT --> Physical Page
    *((uint64_t*)(VMM_PML2_START + (0x200 * 0x200 * pml4idx) + (pml3idx * 0x200) + pml2idx)) = (paddr | flags) | 0b10000000;
    flush_cr3();
    return vaddr;
}
*/

/*
uint64_t vmm_map_vaddr_to_paddr(uint64_t paddr, uint64_t vaddr, uint8_t flags) {
    // Get page struct offsets associated with virtual addresss
    size_t pml4idx = (vaddr & ((uint64_t)0x1ff << 39)) >> 39;
    size_t pml3idx = (vaddr & ((uint64_t)0x1ff << 30)) >> 30;
    size_t pml2idx = (vaddr & ((uint64_t)0x1ff << 21)) >> 21;

    if (pml4idx == 0) {
        P4[pml4idx] = VMM_PML3_INITIAL | 0x3;
        if (pml3idx == 0) {
            P3_start[pml3idx] = VMM_PML2_INITIAL | 0x3;
            P2_start[pml2idx + 0x200 * pml4idx + 0x200 * pml3idx] = (vaddr | flags) | 0b10000000;
        } else {
            P3[pml3idx + 0x200 * pml4idx] = VMM_PML2_START + 0x200 * pml3idx;
            P2[pml2idx + 0x200 * pml4idx + 0x200 * pml3idx] = (vaddr | flags) | 0b10000000;
        }
    } else {
        P4[pml4idx] = VMM_PML3_START + 0x200 * pml4idx;
        if (pml3idx == 0) {
            P3[pml3idx] = VMM_PML2_INITIAL | 0x3;
        } else {
            P3[pml3idx + 0x200 * pml4idx] = VMM_PML2_START + 0x200 * pml3idx;
        }
        P2[pml2idx + 0x200 * pml4idx + 0x200 * pml3idx] = (vaddr | flags) | 0b10000000;
    }
    flush_cr3();
    return vaddr;
}
*/

uint64_t vmm_map_vaddr_to_paddr(uint64_t paddr, uint64_t vaddr, uint8_t flags) {
    // Get page struct offsets associated with virtual addresss
    size_t pml4idx = (vaddr & ((uint64_t)0x1ff << 39)) >> 39;
    size_t pml3idx = (vaddr & ((uint64_t)0x1ff << 30)) >> 30;
    size_t pml2idx = (vaddr & ((uint64_t)0x1ff << 21)) >> 21;

    P4[pml4idx] = (VMM_PML3_START + 0x200 * pml4idx) | 0x3;
    P3[pml3idx + 0x200 * pml4idx] = (VMM_PML2_START + 0x200 * pml3idx) | 0x3;
    P2[pml2idx + 0x200 * pml4idx + 0x200 * pml3idx] = (vaddr | flags) | 0b10000000;

    return vaddr;
}

uint64_t vmm_align_2m(uint64_t paddr) {
    for (size_t i = 0; i < 20; i++) {   // 2^12 = 4K, 2^20 = 2M
        paddr &= ~(1 << i);
    }

    return paddr;
}

uint64_t vmm_align_4k(uint64_t paddr) {
    for (size_t i = 0; i < 12; i++) {
        paddr &= ~(1 << i);
    }

    return paddr;
}

void* vmm_alloc_one_page() {
    uint64_t block = pmm_alloc(1);
    vmm_map_vaddr_to_paddr(block, block, 0x3);
    return (void*)block;
}