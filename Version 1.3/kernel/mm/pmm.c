#include "pmm.h"

void pmm_init(struct stivale_struct *stivale_struct) {
    pmm_read_mmap(stivale_struct);
}

static uint64_t pmm_read_mmap(struct stivale_struct *stivale_struct) {
    struct mmap_entry* memory_map_start = (struct mmap_entry*)stivale_struct->memory_map_addr;
    if (stivale_struct->memory_map_entries > PMM_MMAP_MAX_ENTRIES) {
        panic((uint8_t*)stivale_struct->framebuffer_addr, "PMM - Too many mmap entries.");
    }

    uint64_t total_available_memory = 0;
    for (uint64_t i = 0; i < stivale_struct->memory_map_entries; i++) {
        pmm_mmap[i].base = memory_map_start->base;
        pmm_mmap[i].length = memory_map_start->length;
        pmm_mmap[i].type = memory_map_start->type;
        pmm_mmap[i].unused = memory_map_start->unused;
        if (memory_map_start->type == 1) {
            total_available_memory += memory_map_start->length;
        }
        memory_map_start += sizeof(struct mmap_entry);
    }

    serial_puts("\n[PMM] Total available memory: ");
    serial_puts(unsigned_long_to_str(total_available_memory));
    return total_available_memory;
}