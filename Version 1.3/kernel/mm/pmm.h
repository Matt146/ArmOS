#ifndef PMM_H
#define PMM_H

#include <stddef.h>
#include <stdint.h>
#include "../../stivale.h"
#include "../lib/util.h"
#include "../lib/panic.h"

#define HIGH_VMA 0xffffffff80000000
#define PMM_MMAP_MAX_ENTRIES 256
#define PMM_PAGE_SIZE 0x1000

#define PMM_ALLOC_FAIL 0xFFFFFFFFFFFFFFFF // What is returned by PMM_ALLOC when it fails

#define __PMM_BITMAP_LENGTH (pmm_bitmap.end - (uint64_t)pmm_bitmap.bitmap)
#define __PMM_BITMAP_SIZE (__PMM_BITMAP_LENGTH * 8)

struct bitmap {
    uint8_t* bitmap;    // starting pointer
    uint64_t end;       // ending address
};

static struct bitmap pmm_bitmap;

// Exported functions
void pmm_init(struct stivale_struct *stivale_struct);       // Initialize the PMM - Run this before any other PMM exported function
uint64_t pmm_align_paddr(uint64_t paddr);                   // Aligns a physical address by PMM_PAGE_SIZE
uint64_t pmm_paddr_to_block(uint64_t paddr);                // Convert a physical address to a PMM block
uint64_t pmm_block_to_paddr(uint64_t block);                // Convert a PMM block to a physical address
uint64_t pmm_alloc(uint64_t blocks);
void pmm_free(uint64_t paddr_start, uint64_t blocks);


// Internal state functions
static uint64_t pmm_how_much_memory(struct stivale_struct *stivale_struct);                             // Determines the amount of total usable memory installed
static uint64_t pmm_allocate_bitmap(struct stivale_struct *stivale_struct, uint64_t bitmap_size);       // Returns the address of where to store the bitmap determined by the bitmap_size - returns 0 on error
static void pmm_init_bitmap(struct stivale_struct *stivale_struct, struct bitmap* _bitmap);               // Fills out the bitmap with the correct information
// If block is 1, then it is free. If block is 0, then it is used.
static void pmm_set_bitmap_block_free(struct bitmap* _bitmap, uint64_t block);
static void pmm_set_bitmap_block_used(struct bitmap* _bitmap, uint64_t block);
static bool pmm_block_is_free(struct bitmap* _bitmap, uint64_t block);

#endif // PMM_H