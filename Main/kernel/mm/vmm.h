#ifndef VMM_H
#define VMM_H

#include <stdint.h>
#include <stddef.h>
#include "pmm.h"

// By default, stivale1 protocol mappings at entry will be:
// Base Physical Address -                    Size                    ->  Virtual address
//  0x0000000000000000   - 4 GiB plus any additional memory map entry -> 0x0000000000000000
//  0x0000000000000000   - 4 GiB plus any additional memory map entry -> 0xffff800000000000 (4-level paging only)
//  0x0000000000000000   - 4 GiB plus any additional memory map entry -> 0xff00000000000000 (5-level paging only)
//  0x0000000000000000   -                 0x80000000                 -> 0xffffffff80000000

static uint64_t* p4;

void vmm_map_page(uint64_t vaddr, uint64_t paddr, uint16_t flags);
bool vmm_page_is_mapped(uint64_t vaddr);
void vmm_set_cr3(uint64_t addr);
uint64_t vmm_get_cr3();

#endif // VMM_H