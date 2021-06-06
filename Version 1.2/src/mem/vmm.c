#include "vmm.h"

void set_cr3(uint64_t addr) {
    asm volatile("movq %0,%%cr3": :"r" (addr));
}

void vmm_init() {
    serial_puts("\n\n11111HERE??!??!??!");
    for (unsigned char* i = VMM_PAGE_TABLES_START; i < VMM_PAGE_TABLES_END; i++) {
        //serial_puts(unsigned_long_to_str(i));
        //serial_puts("\n");
        *i = 0;
    }
    serial_puts("\n\n2222HERE??!??!??!");
    for (size_t i = 0; i < 0x200000000; i+=0x200000) {
        vmm_map_vaddr_to_paddr(i, i, 0x3);
    }
    serial_puts("\n\n333333333333333333HERE??!??!??!\n");
    set_cr3(VMM_CR3);
    serial_puts("\n\n44444444444444444444444HERE??!??!??!");
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

    serial_puts("\n[+]Mapping page...");
    serial_puts("\n- Vaddr: ");
    serial_puts(unsigned_long_to_str(vaddr));
    serial_puts("\n - P4 index: ");
    serial_puts(unsigned_long_to_str(pml4idx));
    serial_puts("\n - P3 index: ");
    serial_puts(unsigned_long_to_str(pml3idx));
    serial_puts("\n - P2 index: ");
    serial_puts(unsigned_long_to_str(pml2idx));

    uint64_t p4_index_into = pml4idx;
    uint64_t p4_address = (VMM_PML3_START + 0x200 * pml4idx);
    uint64_t p4_value = p4_address | 0x3;
    P4[p4_index_into] = p4_value;
    serial_puts("\n - Indexing into P4 through: ");
    serial_puts(unsigned_long_to_str(p4_index_into));
    serial_puts("\n - P4 Points To: ");
    serial_puts(unsigned_long_to_str(p4_value));

    uint64_t p3_index_into = pml3idx + 0x200 * pml4idx;
    uint64_t p3_address = (VMM_PML2_START + (0x200 * pml3idx) + (0x200 * 0x200 * pml4idx));
    uint64_t p3_value = p3_address | 0x3;
    P3[p3_index_into] = p3_value;
    serial_puts("\n - Indexing into P3 through: ");
    serial_puts(unsigned_long_to_str(p3_index_into));
    serial_puts("\n - P3 Points To: ");
    serial_puts(unsigned_long_to_str(p3_address));

    uint64_t p2_index_into = pml2idx + (0x200 * 0x200 * pml4idx) + (0x200 * pml3idx);
    uint64_t p2_address = paddr;
    uint64_t p2_value = (p2_address | flags) | 0b10000000;
    P2[p2_index_into] = p2_value;
    serial_puts("\n - Indexing into P2 through: ");
    serial_puts(unsigned_long_to_str(p2_index_into));
    serial_puts("\n - P2 Points To: ");
    serial_puts(unsigned_long_to_str(p2_address));

    return paddr;
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