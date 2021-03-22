#include "pmm.h"

uint64_t pmm_paddr_to_block(uint64_t paddr) {
    if (paddr < BITMAP_START_ADDR) {
        return 0;
    }

    paddr -= BITMAP_START_ADDR;
    return paddr / BITMAP_MEMORY_SIZE;
}

uint64_t pmm_block_to_paddr(uint64_t block) {
    if ((block / 8) > BITMAP_ENTRIES) {
        return 0;
    }

    block *= BITMAP_MEMORY_SIZE;
    block += BITMAP_START_ADDR;
    return block;
}

// If block is 1, then it is free. If block is 0, then it is used.
void pmm_set_bitmap_block_free(uint64_t block) {
    if ((block / 8) < BITMAP_ENTRIES) {
        pmm_bitmap[block / 8] = pmm_bitmap[block / 8] | (1 << (block % 8));
    } else {
        vga_puts("[ERROR] Block / 8 > BITMAP_ENTRIES!\n", VGA_COLOR_YELLOW);
    }
}

void pmm_set_bitmap_block_used(uint64_t block) {
    if ((block / 8) < BITMAP_ENTRIES) {
        pmm_bitmap[block / 8] = pmm_bitmap[block / 8] & ~(1 << (block % 8));
    } else {
        vga_puts("[ERROR] BLOCK / 8 > BITMAP_ENTRIES!\n", VGA_COLOR_YELLOW);
    }
}

bool pmm_block_is_free(uint64_t block) {
    return (pmm_bitmap[block / 8] >> (block % 8)) & 1;
}

void pmm_init_bitmap() {
    vga_puts("[+] Zero initializing memory bitmap.\n", VGA_COLOR_YELLOW);
    for (size_t i = 0; i < BITMAP_ENTRIES; i++) {
        pmm_bitmap[i] = 0;
    }
    vga_puts("[+] Initializing bimap values with BIOS memory map output.\n", VGA_COLOR_YELLOW);
    for (size_t i = 0; i < MEMORY_MAP_ENTRIES_MAX; i++) {
        // If addr range descriptor type is 2, that means it's usable
        // Otherwise, mark it as unusable
        if (pmm_memory_map_entries[i].type == 1) {
            uint64_t start_paddr = pmm_memory_map_entries[i].base_addr;
            uint64_t end_paddr = pmm_memory_map_entries[i].base_addr + pmm_memory_map_entries[i].length;
            // POSSIBLE BUG? - ALLOCATES 1 PAGE PAST THE BITMAP MEMORY SIZE
            for (uint64_t j = start_paddr; j < end_paddr; j+=BITMAP_MEMORY_SIZE) {
                pmm_set_bitmap_block_free(pmm_paddr_to_block(j));
                serial_puts(unsigned_long_to_str(j), VGA_COLOR_YELLOW);
                serial_puts(" ", VGA_COLOR_YELLOW);
                serial_puts(unsigned_long_to_str(pmm_paddr_to_block(j)), VGA_COLOR_LIGHTRED);
                serial_puts(" ", VGA_COLOR_LIGHTRED);
            }
        }
    }
    serial_puts("[+] Bitmap initialized!\n", VGA_COLOR_YELLOW);
    serial_puts("[+] Start of Bitmap Address:", VGA_COLOR_YELLOW);
    serial_puts(unsigned_long_to_str(pmm_bitmap), VGA_COLOR_YELLOW);
}

void pmm_read_bios_memory_map() {
    void* bios_memory_map_ptr = (void*)MEMORY_MAP_LOCATION;
    for (size_t i = 0; i < MEMORY_MAP_ENTRIES_MAX; i++) {
        pmm_memory_map_entries[i].base_addr = *((uint64_t*)(bios_memory_map_ptr));
        bios_memory_map_ptr += sizeof(uint64_t);
        pmm_memory_map_entries[i].length = *((uint64_t*)(bios_memory_map_ptr));
        bios_memory_map_ptr += sizeof(uint64_t);
        pmm_memory_map_entries[i].type = *((uint32_t*)(bios_memory_map_ptr));
        bios_memory_map_ptr += sizeof(uint32_t);
        pmm_memory_map_entries[i].ACPI = *((uint32_t*)(bios_memory_map_ptr));
        bios_memory_map_ptr += sizeof(uint32_t);
    }

    for (size_t i = 0; i < MEMORY_MAP_ENTRIES_MAX; i++) {
        if (pmm_memory_map_entries[i].base_addr != 0 && pmm_memory_map_entries[i].length != 0) {
            vga_puts("BADDR: ", VGA_COLOR_GREEN);
            vga_puts(unsigned_long_to_str(pmm_memory_map_entries[i].base_addr), VGA_COLOR_GREEN);
            vga_puts(" ", VGA_COLOR_GREEN);
            vga_puts("LEN:", VGA_COLOR_GREEN);
            vga_puts(unsigned_long_to_str(pmm_memory_map_entries[i].length), VGA_COLOR_GREEN);
            vga_puts(" ", VGA_COLOR_GREEN);
            vga_puts("TYPE:", VGA_COLOR_GREEN);
            vga_puts(unsigned_long_to_str(pmm_memory_map_entries[i].type), VGA_COLOR_GREEN);
            vga_puts("\n", VGA_COLOR_GREEN);
        }
    }
}

uint64_t pmm_alloc(uint64_t blocks) {
    bool valid_block = false;
    for (size_t i = 0; i < BITMAP_ENTRIES * 8; i++) {
        if (pmm_block_is_free(i)) {
            vga_puts("[+] Found possible free block...\n", VGA_COLOR_LIGHTGREEN);
            for (size_t j = 0; j < blocks; j++) {
                if (pmm_block_is_free(i + j)) {
                    vga_puts("[-]", VGA_COLOR_LIGHTGREEN);
                    vga_puts(unsigned_long_to_str(i + j), VGA_COLOR_LIGHTGREEN);
                    vga_puts("\n", VGA_COLOR_LIGHTGREEN);
                    valid_block = true;
                } else {
                    valid_block = false;
                    break;
                }
            }
            if (valid_block == true) {
                for (size_t z = 0; z < blocks; z++) {
                    pmm_set_bitmap_block_used(i + z);
                }
                vga_puts("--------------------", VGA_COLOR_WHITE);
                return pmm_block_to_paddr(i);
            }
        }
    }

    return PMM_ALLOC_FAIL;
}

void pmm_free(uint64_t paddr_start, uint64_t blocks) {
    for (size_t i = pmm_paddr_to_block(paddr_start); i < blocks; i++) {
        pmm_set_bitmap_block_free(i);
    }
}
