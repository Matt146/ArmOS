#ifndef PMM_H
#define PMM_H

#include <stddef.h>
#include <stdint.h>
#include "../../stivale.h"
#include "../lib/util.h"
#include "../lib/panic.h"

#define KERNEL_HIGH_VMA 0xffffffff80000000
#define HIGH_VMA 0xffff800000000000
#define GLOBAL_OFFSET 0
#define PMM_MMAP_MAX_ENTRIES 256
#define PMM_PAGE_SIZE 0x1000

#define PMM_ALLOC_FAIL 0xFFFFFFFFFFFFFFFF // What is returned by PMM_ALLOC when it fails

#define __PMM_SAFE_ALLOCS_ALWAYS

#define DIV_ROUND_UP(n, d) (((n) + (d) - 1) / (d))

#define __PMM_BITMAP_LENGTH (pmm_bitmap.end - (uint64_t)pmm_bitmap.bitmap)
#define __PMM_BITMAP_SIZE (__PMM_BITMAP_LENGTH * 8)

struct bitmap {
    uint8_t* bitmap;    // starting pointer
    uint64_t end;       // ending address
};

uint64_t total_memory;              // Internal state - stores total memory
static struct bitmap pmm_bitmap;    // Internal state - stores bitmap struct
static uint8_t pmm_bitmap_mux = 0;  // Internal state - PMM mutex

static bool __pmm_initialized = false;

// Exported functions
void pmm_init(struct stivale_struct *stivale_struct);       // Initialize the PMM - Run this before any other PMM exported function
uint64_t pmm_align_paddr(uint64_t paddr);                   // Aligns a physical address by PMM_PAGE_SIZE
uint64_t pmm_paddr_to_block(uint64_t paddr);                // Convert a physical address to a PMM block
uint64_t pmm_block_to_paddr(uint64_t block);                // Convert a PMM block to a physical address
uint64_t pmm_alloc(uint64_t blocks);                        // Returns the physical address of some location in memory that is free with enough memory as specified
void pmm_free(uint64_t paddr_start, uint64_t blocks);       // Marks all blocks from paddr_start (the physical address) to blocks as free


// Internal state functions
static uint64_t pmm_how_much_memory(struct stivale_struct *stivale_struct);                             // Determines the amount of total usable memory installed
static uint64_t pmm_allocate_bitmap(struct stivale_struct *stivale_struct, uint64_t bitmap_size);       // Returns the address of where to store the bitmap determined by the bitmap_size - returns 0 on error
static void pmm_init_bitmap(struct stivale_struct *stivale_struct, struct bitmap* _bitmap);               // Fills out the bitmap with the correct information
// If block is 1, then it is free. If block is 0, then it is used.
static void pmm_set_bitmap_block_free(struct bitmap* _bitmap, uint64_t block);
static void pmm_set_bitmap_block_used(struct bitmap* _bitmap, uint64_t block);
static bool pmm_block_is_free(struct bitmap* _bitmap, uint64_t block);


















/////////////////////////////////////// VMMM FUNCTIONS://///////////////////////////////////
// EDIT: I KNOW THIS IS CRUSTY BUT THIS IS HOW WE GET __PMM_SAFE_ALLOCS_ALWAYS

// By default, stivale1 protocol mappings at entry will be:
// Base Physical Address -                    Size                    ->  Virtual address
//  0x0000000000000000   - 4 GiB plus any additional memory map entry -> 0x0000000000000000
//  0x0000000000000000   - 4 GiB plus any additional memory map entry -> 0xffff800000000000 (4-level paging only)
//  0x0000000000000000   - 4 GiB plus any additional memory map entry -> 0xff00000000000000 (5-level paging only)
//  0x0000000000000000   -                 0x80000000                 -> 0xffffffff80000000

static uint64_t* p4;
static bool __vmm_initialized = false;

void vmm_map_page(uint64_t vaddr, uint64_t paddr, uint16_t flags);
void vmm_unmap_page(uint64_t vaddr);
bool vmm_page_is_mapped(uint64_t vaddr);
void vmm_check_and_map(uint64_t vaddr, uint64_t paddr); // checks if vaddr is mapped and then if it isn't, it maps it to paddr
void vmm_check_and_iden_map(uint64_t vaddr);            // checks if vaddr is mapped and then if it isn't, it is identity mapped
void vmm_set_cr3(uint64_t addr);
void vmm_flush_cr3();
uint64_t vmm_get_cr3();
uint64_t vmm_vaddr_to_paddr(uint64_t vaddr);            // takes the given vaddr and turns it into the corresponding paddr

#endif // PMM_H