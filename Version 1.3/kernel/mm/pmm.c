#include "pmm.h"

void pmm_init(struct stivale_struct *stivale_struct) {
    // Determine the size of the bitmap
    uint64_t total_memory = pmm_how_much_memory(stivale_struct);
    uint64_t bitmap_size = (total_memory / PMM_PAGE_SIZE) / 8;  // size of the bitmap in bytes

    // Allocate area for the bitmap and store it
    uint64_t bitmap_addr = pmm_allocate_bitmap(stivale_struct, bitmap_size);
    if (bitmap_addr == 0) {
        panic(stivale_struct->framebuffer_addr, "PMM - Not enough space to store bitmap.");
    }
    serial_puts("\n[PMM] Bitmap address: ");
    serial_puts(unsigned_long_to_str(bitmap_addr));

    // Initialize the bitmap
    pmm_bitmap.bitmap = bitmap_addr;
    pmm_bitmap.end = bitmap_addr + bitmap_size;
    serial_puts("\n[PMM] Bitmap end: ");
    serial_puts(unsigned_long_to_str(pmm_bitmap.end));
    pmm_init_bitmap(stivale_struct, &pmm_bitmap);
}

uint64_t pmm_paddr_to_block(uint64_t paddr) {
    /* DEBUG:
    serial_puts("\n[PMM] PADDR -> BLOCK: ");
    serial_puts(unsigned_long_to_str(paddr));
    serial_puts(" -> ");
    serial_puts(unsigned_long_to_str(paddr / PMM_PAGE_SIZE));
    */
    return paddr / PMM_PAGE_SIZE;
}

uint64_t pmm_block_to_paddr(uint64_t block) {
    /* DEBUG:
    serial_puts("\n[PMM] BLOCK -> PADDR: ");
    serial_puts(unsigned_long_to_str(block));
    serial_puts(" -> ");
    serial_puts(unsigned_long_to_str(block * PMM_PAGE_SIZE));
    */
    return block * PMM_PAGE_SIZE;
}

uint64_t pmm_align_paddr(uint64_t paddr) {
    return (paddr & ~(PMM_PAGE_SIZE-1)) + PMM_PAGE_SIZE;
}

static bool pmm_block_is_free(struct bitmap* _bitmap, uint64_t block) {
    return (_bitmap->bitmap[block / 8] >> (block % 8)) & 1;
}

uint64_t pmm_alloc(uint64_t blocks) {
    bool valid_block = false;
    serial_puts("\n[PMM] Allocating blocks...");
    serial_puts("\n - [-] Number of blocks being allocated: ");
    serial_puts(unsigned_long_to_str(blocks));
    for (size_t i = 0; i < __PMM_BITMAP_SIZE * 8; i++) {
        if (pmm_block_is_free(&pmm_bitmap, i)) {
            serial_puts("\n - [-] Found possible free block...");
            for (size_t j = 0; j < blocks; j++) {
                if (pmm_block_is_free(&pmm_bitmap, i + j)) {
                    serial_puts("\n - [-]");
                    serial_puts(unsigned_long_to_str(i + j));
                    valid_block = true;
                } else {
                    valid_block = false;
                    break;
                }
            }
            if (valid_block == true) {
                for (size_t z = 0; z < blocks; z++) {
                    pmm_set_bitmap_block_used(&pmm_bitmap, i + z);
                }
                serial_puts("\n--------------------");
                return pmm_block_to_paddr(i);
            }
        }
    }

    return PMM_ALLOC_FAIL;
}

void pmm_free(uint64_t paddr_start, uint64_t blocks) {
    serial_puts("\n[PMM] Freeing blocks...");
    serial_puts("\n - [-] Starting block: ");
    serial_puts(unsigned_long_to_str(pmm_paddr_to_block(paddr_start)));
    serial_puts("\n - [-] Ending block: ");
    serial_puts(unsigned_long_to_str(pmm_paddr_to_block(paddr_start) + blocks));
    for (uint64_t i = pmm_paddr_to_block(paddr_start); i < blocks + pmm_paddr_to_block(paddr_start); i++) {
        serial_puts("\n - [-]");
        serial_puts(unsigned_long_to_str(i));
        pmm_set_bitmap_block_free(&pmm_bitmap, i);
    }
    serial_puts("\n--------------------");
}


static uint64_t pmm_how_much_memory(struct stivale_struct *stivale_struct) {
    uint64_t total_available_memory = 0;
    struct stivale_mmap_entry* memory_map_start = (struct stivale_mmap_entry*)stivale_struct->memory_map_addr;
    for (uint64_t i = 0; i < stivale_struct->memory_map_entries; i++) {
        total_available_memory += memory_map_start[i].length;
        serial_puts("\n[PMM] MMAP Entry Base: ");
        serial_puts(unsigned_long_to_str(memory_map_start[i].base));
        serial_puts("\n[PMM] MMAP Entry Length: ");
        serial_puts(unsigned_long_to_str(memory_map_start[i].length));
        serial_puts("\n[PMM] MMAP Entry Type: ");
        serial_puts(unsigned_long_to_str(memory_map_start[i].type));
    }
    serial_puts("\n[PMM] Total available memory:");
    serial_puts(unsigned_long_to_str(total_available_memory));
    return total_available_memory;
}

static uint64_t pmm_allocate_bitmap(struct stivale_struct *stivale_struct, uint64_t bitmap_size) {
    // Loop through looking for the first valid entry that we can use to store our bitmap
    struct stivale_mmap_entry* memory_map_start = (struct stivale_mmap_entry*)stivale_struct->memory_map_addr;
    uint64_t mmap_addr = 0;
    for (uint64_t i = 0; i < stivale_struct->memory_map_entries; i++) {
        if (memory_map_start[i].type == 1 && memory_map_start[i].length >= bitmap_size) {
            mmap_addr = memory_map_start[i].base;
            break;
        }
    }

    if (memory_map_start[0].base == stivale_struct->memory_map_addr && memory_map_start[0].length < bitmap_size) {
        return 0;
    }
    return (uint64_t)mmap_addr;
}

// If block is 1, then it is free. If block is 0, then it is used.
static void pmm_set_bitmap_block_free(struct bitmap* _bitmap, uint64_t block) {
    if ((block / 8) < __PMM_BITMAP_SIZE) {
        _bitmap->bitmap[block / 8] = _bitmap->bitmap[block / 8] | (1 << (block % 8));
    } else {
        serial_puts("\n[PMM] ERROR - Unable to set bitmap block to free.");
    }
}

static void pmm_set_bitmap_block_used(struct bitmap* _bitmap, uint64_t block) {
    if ((block / 8) < __PMM_BITMAP_SIZE) {
        _bitmap->bitmap[block / 8] = _bitmap->bitmap[block / 8] & ~(1 << (block % 8));
    } else {
        serial_puts("\n[PMM] ERROR - Unable to set bitmap block to used.");
    }
}

static void pmm_init_bitmap(struct stivale_struct *stivale_struct, struct bitmap* _bitmap) {
    // Memset the bitmap area to 0
    memsetb(_bitmap->bitmap, 0, __PMM_BITMAP_LENGTH);
    serial_puts("\n[PMM] Bitmap memset to 0.");

    // Fill out the bitmap
    struct stivale_mmap_entry* memory_map_start = (struct stivale_mmap_entry*)stivale_struct->memory_map_addr;
    for (uint64_t i = 0; i < stivale_struct->memory_map_entries; i++) {
        if (memory_map_start[i].type == 1) {
            uint64_t start_paddr = memory_map_start[i].base;
            uint64_t end_paddr = memory_map_start[i].base + memory_map_start[i].length;
            // POSSIBLE BUG? - ALLOCATES 1 PAGE PAST THE BITMAP MEMORY SIZE
            for (uint64_t j = start_paddr; j < end_paddr; j+=PMM_PAGE_SIZE) {
                pmm_set_bitmap_block_free(_bitmap, pmm_paddr_to_block(j));
            }
        }
    }

    // Mark the bitmap area itself as used
    for (uint64_t i = _bitmap->bitmap; i < _bitmap->end; i++) {
        pmm_set_bitmap_block_used(_bitmap, pmm_paddr_to_block(i));
    }
}