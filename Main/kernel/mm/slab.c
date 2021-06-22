#include "slab.h"

void mm_slab_init() {
    struct mm_slab_cache* mm_slab_cache = (struct mm_slab_cache*)(pmm_alloc(1) + KERNEL_HIGH_VMA);
    memsetb((uint8_t*)mm_slab_cache, 0, PMM_PAGE_SIZE);

    mm_slab_cache->cache_name = "ROOT";
    mm_slab_cache->slabs_full = NULL;
    mm_slab_cache->slabs_partial = NULL;
    mm_slab_cache->slabs_free = NULL;
    mm_slab_cache->flags = 0;
    mm_slab_cache->object_size = 0;
    mm_slab_cache->batchcount = SLAB_STD_BATCHCOUNT;
    mm_slab_cache->slab_mu = 0;
    mm_slab_cache->next = NULL;
    mm_slab_cache->last = NULL;

    // Now add custom caches
    mm_slab_create_cache("Chad", 0, 8);
    mm_slab_create_cache("Chad", 0, 16);
    mm_slab_create_cache("Chad", 0, 32);
    mm_slab_create_cache("Chad", 0, 64);
    mm_slab_create_cache("Chad", 0, 128);
    mm_slab_create_cache("Chad", 0, 256);
    mm_slab_create_cache("Chad", 0, 1024);
    mm_slab_create_cache("Chad", 0, 2048);
    mm_slab_create_cache("Chad", 0, 4096);
    mm_slab_create_cache("Chad", 0, 8192);
    mm_slab_create_cache("Chad", 0, 16384);
    mm_slab_create_cache("Chad", 0, 32768);
}

static void mm_alloc_slabs(struct mm_slab_cache* cache) {
    struct mm_slab* start = cache->slabs_free;
    while (start->next != NULL) {
        start = start->next;
    }

    start->next = (struct mm_slab*)(pmm_alloc(DIV_ROUND_UP(cache->batchcount * sizeof(struct mm_slab), PMM_PAGE_SIZE)) + KERNEL_HIGH_VMA);
    memsetb((uint8_t*)start->next, 0, DIV_ROUND_UP(cache->batchcount * sizeof(struct mm_slab), PMM_PAGE_SIZE));

    if (!vmm_page_is_mapped((uint64_t)start->next)) {
        vmm_map_page((uint64_t)start->next, (uint64_t)start->next - KERNEL_HIGH_VMA, 0x3);
    }

    for (size_t i = 0; i < cache->batchcount; i++) {
        start->next->free = cache->batchcount;
        start->next->inuse = 0;
        start->next->next = NULL;
        start->next->last = start;

        start->next->buff = (void*)(pmm_alloc(DIV_ROUND_UP(cache->batchcount * sizeof(struct mm_slab), PMM_PAGE_SIZE)) + KERNEL_HIGH_VMA);
        start->next->bitmap = (uint8_t*)(pmm_alloc(DIV_ROUND_UP(cache->object_size * cache->batchcount, 8)) + KERNEL_HIGH_VMA);

        if (!vmm_page_is_mapped((uint64_t)start->next->buff)) {
            vmm_map_page((uint64_t)start->next->buff, (uint64_t)start->next->buff - KERNEL_HIGH_VMA, 0x3);
        }

        if (!vmm_page_is_mapped((uint64_t)start->next->bitmap)) {
            vmm_map_page((uint64_t)start->next->buff, (uint64_t)start->next->buff - KERNEL_HIGH_VMA, 0x3);
        }

        start->next += sizeof(struct mm_slab);
    }
}

struct mm_slab_cache* mm_slab_create_cache(const char* name, uint32_t flags, uint64_t object_size) {
    struct mm_slab_cache* start = root_cache;
    while (start->next != NULL) {
        start = start->next;
    }

    start->next = (struct mm_slab_cache*)(pmm_alloc(1) + KERNEL_HIGH_VMA);
    start->next->cache_name = name;
    start->next->slabs_full = NULL;
    start->next->slabs_partial = NULL;
    start->next->slabs_free = NULL;
    start->next->flags = flags;
    start->next->object_size = object_size;
    start->next->batchcount = SLAB_STD_BATCHCOUNT;
    start->next->slab_mu = 0;
    start->next->slab_mu = 0;
    start->next->next = NULL;
    start->next->last = start;

    // Continue with the free slabs
    mm_alloc_slabs(start->next);
    return start->next;
}

static bool mm_slab_block_is_free(uint8_t* bitmap, uint64_t i) {
    return (bitmap[i / 8] >> (i % 8)) & 1;
}

static void mm_slab_set_block_used(uint8_t* bitmap, uint64_t i) {
    bitmap[i / 8] = bitmap[i / 8] | (1 << (i % 8));
}

static uint64_t mm_slab_find_free_object(uint8_t* bitmap, uint64_t starting_addr, uint64_t obj_size, uint64_t objects_to_allocate, uint64_t possible_objects_in_bitmap) {
    if (objects_to_allocate == 0) {
        return __MM_FIND_FREE_OBJECT_FAIL;
    }

    bool valid_block = false;
    for (size_t i = 0; i < possible_objects_in_bitmap * 8; i++) {
        if (mm_slab_block_is_free(bitmap, i)) {
            //serial_puts("\n - [-] Found possible free block...");
            for (size_t j = 0; j < objects_to_allocate; j++) {
                if (mm_slab_block_is_free(bitmap, i + j)) {
                    valid_block = true;
                } else {
                    valid_block = false;
                    break;
                }
            }
            if (valid_block == true) {
                for (size_t z = 0; z < objects_to_allocate; z++) {
                    mm_slab_set_block_used(bitmap, i + z);
                }
                //serial_puts("\n--------------------");
                return starting_addr;
            }
        }
    }

    return __MM_FIND_FREE_OBJECT_FAIL;
}

static void mm_move_free_slabs_to_partial_list(struct mm_slab_cache* cache, uint64_t num) {
    // Find the last slabs_partial and set the first slabs_free to its next slab
    // Also, set the last slab_free's last to the last slab_partial.
    struct mm_slab* slabs_partial_last = cache->slabs_partial;
    for (size_t i = 0; i < num; i++) {
        if (slabs_partial_last->next == NULL) {
            serial_puts("\n[SLAB] mm_move_free_slabs_to_partial_list failed...");
            return;
        }

        slabs_partial_last = slabs_partial_last->next;
    }
    slabs_partial_last->next = cache->slabs_free;
    cache->slabs_free->last = slabs_partial_last;

    // Set the first slab_free to the next of the last slab_free
    // Find the last slab_free and set its next to NULL
    struct mm_slab* slabs_free_last = cache->slabs_free;
    for (size_t i = 0; i < num; i++) {
        if (slabs_free_last->next == NULL) {
            serial_puts("\n[SLAB] 2nd: mm_move_free_slabs_to_partial_list failed...");
            return;
        }
        slabs_free_last = slabs_free_last->next;
    }
    cache->slabs_free = slabs_free_last;
    slabs_free_last->next = NULL;

    //slabs_partial_last->next = slabs_partial_last->last;
}

void* kmalloc(uint64_t size) {
    // Get the cache with suitable-sized objects
    struct mm_slab_cache* start = root_cache;
    while (start->next != NULL) {
        if (start->object_size >= size) {
            break;
        }
        start = start->next;
    }

    if (start->object_size == root_cache->object_size) {
        // No suitable cache found
        serial_puts("\n[SLAB] ALLOC FAILED!");
        return NULL;
    }

    // Try to find a free one based off of slabs_partial
    struct mm_slab* slab_start = start->slabs_partial;
    while (slab_start->next != NULL) {
        uint64_t free_obj = mm_find_free_object(slab_start->bitmap, (uint64_t)slab_start->buff, start->object_size, DIV_ROUND_UP(size, start->object_size), start->batchcount);
        if (free_obj != __MM_FIND_FREE_OBJECT_FAIL) {
            return (void*)free_obj;
        }

        slab_start = slab_start->next;
    }

    // Otherwise, allocate slabs, move batchcount slabs to slabs_partial and start again
    mm_alloc_slabs(start);
    mm_move_free_slabs_to_partial_list(start, start->batchcount);
}