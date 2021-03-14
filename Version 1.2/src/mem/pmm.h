#ifndef ALLOC_H
#define ALLOC_H

#include <stdint.h>
#include <stdbool.h>
#include "../driver/vga/vga.h"

#define PML4T   0x15000
#define PDPT    0x16000
#define PDT     0x17000
#define STACK_BASE 0x90000
#define STACK_TOP get_rsp()
#define MEMORY_MAP_LOCATION 0x7E00
#define MEMORY_MAP_LOCATION_END 0x8FFF
#define MEMORY_MAP_ENTRY_SIZE 24
#define MEMORY_MAP_ENTRIES_MAX ((MEMORY_MAP_LOCATION_END - MEMORY_MAP_LOCATION) / MEMORY_MAP_ENTRY_SIZE)
#define BITMAP_START_ADDR 0x100000  // The memory address where the bitmap starts representing from.
#define BITMAP_ENTRIES 0x1000       // Number of single byte entries in a bitmap. Each byte entry has 8 bits, used to represent BITMAP_MEMORY_SIZE bytes each
#define BITMAP_MEMORY_SIZE 0x200000 // The size of memory each bit in a bitmap byte entry points to - the size of each page basically, as each bit represents 1 page
#define PMM_ALLOC_FAIL 0xFFFFFFFFFFFFFFFF // What is returned by PMM_ALLOC when it fails

// Specifies an address range descriptor
struct addr_range_descriptor {
    uint64_t base_addr;
    uint64_t length;
    uint32_t type;
    uint32_t ACPI;
} __attribute__((packed));

struct addr_range_descriptor pmm_memory_map_entries[MEMORY_MAP_ENTRIES_MAX];
void pmm_read_bios_memory_map();

// Bitmap initialization stuff
uint8_t pmm_bitmap[BITMAP_ENTRIES];
void pmm_init_bitmap();

// Block abstraction
uint64_t pmm_paddr_to_block(uint64_t paddr);    // gets the bit in the memory map
uint64_t pmm_block_to_paddr(uint64_t block);    // go from block --> paddr - WARNING: UNTESTED!
void pmm_set_bitmap_block_free(uint64_t block); // sets the block in the memory map to used
void pmm_set_bitmap_block_used(uint64_t block); // sets the block in the memory map to free - WARNING: UNTESTED!
bool pmm_block_is_free(uint64_t block);         // checks whether a block in the memory map is free - WARNING: UNTESTED!

// Allocation stuff
uint64_t pmm_alloc(uint64_t blocks);                // allocates "blocks" blocks
void pmm_free(uint64_t paddr_start, uint64_t blocks); // frees memory from paddr_start up to "blocks" blocks

extern uint64_t get_rsp();
extern void flush_cr3(); // By flushing (reloading) CR3, we can flush the TLB entries - We need to do this every time we update PTE's.

#endif // ALLOC_H