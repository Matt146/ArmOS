#include "vmm.h"


// By default, stivale1 protocol mappings at entry will be:
// Base Physical Address -                    Size                    ->  Virtual address
//  0x0000000000000000   - 4 GiB plus any additional memory map entry -> 0x0000000000000000
//  0x0000000000000000   - 4 GiB plus any additional memory map entry -> 0xffff800000000000 (4-level paging only)
//  0x0000000000000000   - 4 GiB plus any additional memory map entry -> 0xff00000000000000 (5-level paging only)
//  0x0000000000000000   -                 0x80000000                 -> 0xffffffff80000000

void vmm_set_cr3(uint64_t addr) {
    asm volatile("movq %0,%%cr3": :"r" (addr));
}

void vmm_init() {
    // Allocate the P4
    p4 = (uint64_t*)pmm_alloc(1);
    memsetb(p4, 0, 0x1000);

    // Map new page tables with the old limine mappings
    for (uint64_t i = 0; i < 0x100000000; i+=0x1000) {
        vmm_map_page(i, i, 0x3);
    }

    uint64_t i = 0;
    for (uint64_t j = 0xffff800000000000; j < 0x100000000 + 0xffff800000000000; j+=0x1000) {
        vmm_map_page(j, i, 0x3);
        i += 0x1000;
    }

    i = 0;
    for (uint64_t j = 0xffffffff80000000; j < 0xfffffffffffff000; j+=0x1000) {
        vmm_map_page(j, i, 0x3);
        i += 0x1000;
    }
    // Set cr3 now
    serial_puts(unsigned_long_to_str(p4));
    vmm_set_cr3((uint64_t)p4);
}

static uint8_t vmm_mux;
void vmm_map_page(uint64_t vaddr, uint64_t paddr, uint16_t flags) {
    // Get the indices of each page table at each level for the virtual address
    uint64_t p4_idx = (vaddr >> 39) & 0x1FF;
    uint64_t p3_idx = (vaddr >> 30) & 0x1FF;
    uint64_t p2_idx = (vaddr >> 21) & 0x1FF;
    uint64_t p1_idx = (vaddr >> 12) & 0x1FF;

    uint64_t* p3 = NULL;
    uint64_t* p2 = NULL;
    uint64_t* p1 = NULL;

    mutex_lock(&vmm_mux);
    // The specific P4 entry does not exist, meaning we need to make a new P3
    // page table and connect the two
    if (p4[p4_idx] == NULL) {
        p3 = (uint64_t*)pmm_alloc(1);
        memsetb(p3, 0, 0x1000);
        p4[p4_idx] = (uint64_t)p3 | 0x3;  // Or with 0x3 to mark page as present and readable/writable
    } else {
        // Else, a P4 entry does exist, so we just set our p3 pointer to the existing
        // lower-level page table
        p3 = (uint64_t*)(p4[p4_idx]  & ~(0xfff));
    }

    // The specific P3 entry does not exist, meaning we need to make a new P2
    // page table and connect the two
    if (p3[p3_idx] == NULL) {
        p2 = (uint64_t*)pmm_alloc(1);
        memsetb(p2, 0, 0x1000);
        p3[p3_idx] = (uint64_t)p2 | 0x3;
    } else {
        // Else, a P3 entry does exist, so just set our p2 pointer to the existing
        // lower-level page table
        p2 = (uint64_t*)(p3[p3_idx]  & ~(0xfff));
    }

    // The specific P2 entry does not exist, meaning we need to make a new P1
    // page table and connect the two
    if (p2[p2_idx] == NULL) {
        p1 = (uint64_t*)pmm_alloc(1);
        memsetb(p1, 0, 0x1000);
        p2[p2_idx] = (uint64_t)p1 | 0x3;
    } else {
        // Else, a P2 entry does exist, so just set our p1 pointer to the existing
        // lower-level page table
        p1 = (uint64_t*)(p2[p2_idx]  & ~(0xfff));
    }

    // Now, we just map the page to the correct physical page-frame
    p1[p1_idx] = paddr | flags;
    mutex_unlock(&vmm_mux);
}

bool vmm_page_is_mapped(uint64_t vaddr) {
    // Get the indices of each page table at each level for the virtual address
    uint64_t p4_idx = (vaddr >> 39) & 0x1FF;
    uint64_t p3_idx = (vaddr >> 30) & 0x1FF;
    uint64_t p2_idx = (vaddr >> 21) & 0x1FF;
    uint64_t p1_idx = (vaddr >> 12) & 0x1FF;

    uint64_t* p3 = NULL;
    uint64_t* p2 = NULL;
    uint64_t* p1 = NULL;

    mutex_lock(&vmm_mux);
    serial_puts("\n\n[VMM] Testing if page ");
    serial_puts(unsigned_long_to_str(vaddr));
    serial_puts(" is mapped");
    serial_puts("\n - P4: ");
    serial_puts(unsigned_long_to_str(p4[p4_idx]));
    if (p4[p4_idx] == NULL || ((p4[p4_idx] & 0x1) != 1)) {
        serial_puts("\n - FAILED ON P4");
        mutex_unlock(&vmm_mux);
        return false;
    }
    p3 = (uint64_t*)(p4[p4_idx] & ~(0xfff));

    if (p3[p3_idx] == NULL || ((p3[p3_idx] & 0x1) != 1)) {
        serial_puts("\n - FAILED ON P3");
        serial_puts("\n - P3: ");
        serial_puts(unsigned_long_to_str(p3[p3_idx]));
        mutex_unlock(&vmm_mux);
        return false;
    }
    p2 = (uint64_t*)(p3[p3_idx] & ~(0xfff));

    if (p2[p2_idx] == NULL|| ((p2[p2_idx] & 0x1) != 1)) {
        serial_puts("\n - FAILED ON P2");
        mutex_unlock(&vmm_mux);
        return false;
    }
    p1 = (uint64_t*)(p2[p2_idx] & ~(0xfff));

    if ((p1[p1_idx] == NULL) || ((p1[p1_idx] & 0x1) != 1)) {
        serial_puts("\n - FAILED ON P1");
        serial_puts("\n - P1: ");
        serial_puts(unsigned_long_to_str(p1));
        serial_puts("\n - P1[p1_idx]: ");
        serial_puts(unsigned_long_to_str(p1[p1_idx]));
        mutex_unlock(&vmm_mux);
        return false;
    }

    mutex_unlock(&vmm_mux);
    return true;
}

void vmm_unmap_page(uint64_t vaddr) {
    // Get the indices of each page table at each level for the virtual address
    uint64_t p4_idx = (vaddr >> 39) & 0x1FF;
    uint64_t p3_idx = (vaddr >> 30) & 0x1FF;
    uint64_t p2_idx = (vaddr >> 21) & 0x1FF;
    uint64_t p1_idx = (vaddr >> 12) & 0x1FF;

    uint64_t* p3 = NULL;
    uint64_t* p2 = NULL;
    uint64_t* p1 = NULL;

    if (vmm_page_is_mapped(vaddr)) {
        mutex_lock(&vmm_mux);
        // The specific P4 entry does not exist, meaning we need to make a new P3
        // page table and connect the two
        if (p4[p4_idx] == NULL) {
            p3 = (uint64_t*)pmm_alloc(1);
            memsetb(p3, 0, 0x1000);
            p4[p4_idx] = (uint64_t)p3 | 0x3;  // Or with 0x3 to mark page as present and readable/writable
        } else {
            // Else, a P4 entry does exist, so we just set our p3 pointer to the existing
            // lower-level page table
            p3 = (uint64_t*)(p4[p4_idx]  & ~(0xfff));
        }

        // The specific P3 entry does not exist, meaning we need to make a new P2
        // page table and connect the two
        if (p3[p3_idx] == NULL) {
            p2 = (uint64_t*)pmm_alloc(1);
            memsetb(p2, 0, 0x1000);
            p3[p3_idx] = (uint64_t)p2 | 0x3;
        } else {
            // Else, a P3 entry does exist, so just set our p2 pointer to the existing
            // lower-level page table
            p2 = (uint64_t*)(p3[p3_idx]  & ~(0xfff));
        }

        // The specific P2 entry does not exist, meaning we need to make a new P1
        // page table and connect the two
        if (p2[p2_idx] == NULL) {
            p1 = (uint64_t*)pmm_alloc(1);
            memsetb(p1, 0, 0x1000);
            p2[p2_idx] = (uint64_t)p1 | 0x3;
        } else {
            // Else, a P2 entry does exist, so just set our p1 pointer to the existing
            // lower-level page table
            p1 = (uint64_t*)(p2[p2_idx]  & ~(0xfff));
        }

        // Now, we just map the page to the correct physical page-frame
        p1[p1_idx] = p1[p1_idx] & 0xFFFFFFFFFFFFFFFE;
    }

    mutex_unlock(&vmm_mux);
}

void vmm_flush_cr3() {
    vmm_set_cr3(vmm_get_cr3());
}

uint64_t vmm_get_cr3() {
    return (uint64_t)p4;
}