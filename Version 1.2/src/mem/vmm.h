#ifndef VMM_H
#define VMM_H

#include <stdint.h>
#include "pmm.h"
#include "../driver/vga/vga.h"
#include "../driver/serial/serial.h"

#define VMM_PAGE_TABLES_START       0x100000  // This is the start of the address space that we can use for paging structures
#define VMM_PAGE_TABLES_END         BITMAP_START_ADDR    // This is the end of the address space that we can use for paging structures
#define VMM_CR3                     VMM_PAGE_TABLES_START

#define VMM_PML4                    VMM_CR3

#define VMM_PML3_INITIAL            0x16000
#define VMM_PML3_START              VMM_PAGE_TABLES_START + 0x1000
#define VMM_PML3_END                VMM_PAGE_TABLES_START + 0x1100000 + 0x1000

#define VMM_PML2_INITIAL            0x17000
#define VMM_PML2_START              VMM_PML3_END + 0x1000
#define VMM_PML2_END                BITMAP_START_ADDR

#define VMM_PAGE_SIZE               BITMAP_MEMORY_SIZE

static uint64_t* P4 = VMM_CR3;
static uint64_t* P3 = VMM_PML3_START;
static uint64_t* P2 = VMM_PML2_START;  // the PDT we use for the bootloader is at 0x17000 (we identity map the first 1 gib)

static uint64_t* P3_start = VMM_PML3_START;
static uint64_t* P2_start = VMM_PML2_START;

void vmm_init();
uint64_t vmm_map_vaddr_to_paddr(uint64_t paddr, uint64_t vaddr, uint8_t flags);
void map_page(uint64_t vaddr, uint64_t paddr, uint64_t flags);   // DEPRACATED
uint64_t vmm_align_2m(uint64_t paddr);
uint64_t vmm_align_4k(uint64_t paddr);

extern void set_cr3(uint64_t addr);

void* vmm_alloc_one_page();

#endif // VMM_H
