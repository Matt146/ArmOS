#include "slab.h"

void mm_slab_init() {
    serial_puts("\n[SLAB] ALlocating root cache and initializing memory...");
    struct mm_slab_cache* _mm_slab_cache = (struct mm_slab_cache*)(pmm_alloc(1) + __NONE);
    memsetb((uint8_t*)_mm_slab_cache, 0, PMM_PAGE_SIZE);

    _mm_slab_cache->cache_name = "ROOT";
    _mm_slab_cache->slabs_full = NULL;
    _mm_slab_cache->slabs_partial = NULL;
    _mm_slab_cache->slabs_free = NULL;
    _mm_slab_cache->flags = 0;
    _mm_slab_cache->object_size = 0;
    _mm_slab_cache->batchcount = SLAB_STD_BATCHCOUNT;
    _mm_slab_cache->slab_mu = 0;
    _mm_slab_cache->next = NULL;
    _mm_slab_cache->last = NULL;
    root_cache = _mm_slab_cache;
    serial_puts("\n[SLAB] Root cache allocated and memory initialized.");

    // Now add custom caches
    serial_puts("\n[SLAB] Adding custom caches...");
    mm_slab_create_cache("Chad", 0, 8);
    mm_slab_create_cache("Chad", 0, 16);
    mm_slab_create_cache("Chad", 0, 32);
    mm_slab_create_cache("Chad", 0, 64);
    mm_slab_create_cache("Chad", 0, 128);
    mm_slab_create_cache("Chad", 0, 256);
    mm_slab_create_cache("Chad", 0, 256);
    serial_puts("\n[SLAB] Custom caches added.");
}

struct mm_slab_cache* mm_slab_create_cache(const char* name, uint32_t flags, uint64_t object_size) {
    //serial_puts("\n[SLAB] mm_slab_create_cache: Finding last cache in linked list.");
    uint64_t num = 0;
    struct mm_slab_cache* start = root_cache;
    while (start->next != NULL) {
        start = start->next;
        serial_puts("\n[SLAB] mm_slab_create_cache: Last cache count: ");
        serial_puts(unsigned_long_to_str(num));
        num++;
    }
    //serial_puts("\n[SLAB] mm_slab_create_cache: Got last cache.");

    void* slabs_full_buff = pmm_alloc(DIV_ROUND_UP(1 * sizeof(struct mm_slab), PMM_PAGE_SIZE)) + __NONE;
    memsetb((uint8_t*)slabs_full_buff, 0, 1 * sizeof(struct mm_slab) * PMM_PAGE_SIZE);
    void* slabs_partial_buff = pmm_alloc(DIV_ROUND_UP(1 * sizeof(struct mm_slab), PMM_PAGE_SIZE)) + __NONE;
    memsetb((uint8_t*)slabs_partial_buff, 0, 1 * sizeof(struct mm_slab) * PMM_PAGE_SIZE);
    void* slabs_free_buff = pmm_alloc(DIV_ROUND_UP(1 * sizeof(struct mm_slab), PMM_PAGE_SIZE)) + __NONE;
    memsetb((uint8_t*)slabs_free_buff, 0, 1 * sizeof(struct mm_slab) * PMM_PAGE_SIZE);

    start->next = (struct mm_slab_cache*)(pmm_alloc(1) + __NONE);
    start->next->cache_name = name;
    start->next->slabs_full = slabs_full_buff;
    start->next->slabs_partial = slabs_partial_buff;
    start->next->slabs_free = slabs_free_buff;
    start->next->flags = flags;
    start->next->object_size = object_size;
    start->next->batchcount = SLAB_STD_BATCHCOUNT;
    start->next->slab_mu = 0;
    start->next->slab_mu = 0;
    start->next->next = NULL;
    start->next->last = start;

    start->next->slabs_full->parent_cache = start->next;
    start->next->slabs_partial->parent_cache = start->next;
    start->next->slabs_free->parent_cache = start->next;

    start->next->slabs_full->buff = pmm_alloc(DIV_ROUND_UP(object_size * 5, PMM_PAGE_SIZE)) + __NONE;
    memsetb((uint8_t*)start->next->slabs_full->buff, 0, object_size * 5 * PMM_PAGE_SIZE);
    start->next->slabs_full->bitmap = pmm_alloc(DIV_ROUND_UP(DIV_ROUND_UP(object_size * 5, 8), PMM_PAGE_SIZE) + __NONE);
    memsetb((uint8_t*)start->next->slabs_full->bitmap, 0xff, DIV_ROUND_UP(object_size * 5 * PMM_PAGE_SIZE, 8));

    start->next->slabs_partial->buff = pmm_alloc(DIV_ROUND_UP(object_size * 5, PMM_PAGE_SIZE)) + __NONE;
    memsetb((uint8_t*)start->next->slabs_partial->buff, 0, object_size * 5 * PMM_PAGE_SIZE);
    start->next->slabs_partial->bitmap = pmm_alloc(DIV_ROUND_UP(DIV_ROUND_UP(object_size * 5, 8), PMM_PAGE_SIZE) + __NONE);
    memsetb((uint8_t*)start->next->slabs_partial->bitmap, 0xff, DIV_ROUND_UP(object_size * 5 * PMM_PAGE_SIZE, 8));

    start->next->slabs_free->buff = pmm_alloc(DIV_ROUND_UP(object_size * 5, PMM_PAGE_SIZE)) + __NONE;
    memsetb((uint8_t*)start->next->slabs_free->buff, 0, object_size * 5 * PMM_PAGE_SIZE);
    start->next->slabs_free->bitmap = pmm_alloc(DIV_ROUND_UP(DIV_ROUND_UP(object_size * 5, 8), PMM_PAGE_SIZE) + __NONE);
    memsetb((uint8_t*)start->next->slabs_free->bitmap, 0xff, DIV_ROUND_UP(object_size * 5 * PMM_PAGE_SIZE, 8));

    // Continue with the free slabs
    //serial_puts("\n[SLAB] mm_slab_create_cache: allocating initial slabs for cache...");
    mm_alloc_slabs(start->next, start->next->batchcount);
    mm_alloc_slabs(start->next, start->next->batchcount);
    //serial_puts("\n[SLAB] mm_slab_create_cache: allocated initial slabs for cache.");
    return start->next;
}

static void mm_append_slabs(struct mm_slab* dstslab, struct mm_slab* srcslab) {
    while (true) {
        if (dstslab->next == NULL) {
            srcslab->last = dstslab;
            srcslab->next = NULL;
            dstslab->next = srcslab;
            //mm_slab_debug_slabs(dstslab);
            break;
        }
        dstslab = dstslab->next;
    }
}

static void mm_alloc_slabs(struct mm_slab_cache* cache, uint64_t count) {
    // Alloc a buffer with enough slabs
    void* buff = pmm_alloc(DIV_ROUND_UP(count * sizeof(struct mm_slab), PMM_PAGE_SIZE)) + __NONE;
    memsetb((uint8_t*)buff, 0, count * sizeof(struct mm_slab) * PMM_PAGE_SIZE);

    // Now, initialize those slabs
    struct mm_slab* slab = (struct mm_slab*)buff;
    for (uint64_t i = 0; i < count; i++) {
        // Parent cache
        slab->parent_cache = cache;

        // Buffer shit
        slab->buff = pmm_alloc(DIV_ROUND_UP(cache->object_size * count, PMM_PAGE_SIZE)) + __NONE;
        memsetb((uint8_t*)slab->buff, 0, cache->object_size * count * PMM_PAGE_SIZE);
        slab->bitmap = pmm_alloc(DIV_ROUND_UP(DIV_ROUND_UP(cache->object_size * count, 8), PMM_PAGE_SIZE) + __NONE);
        memsetb((uint8_t*)slab->bitmap, 0xff, DIV_ROUND_UP(cache->object_size * count * PMM_PAGE_SIZE, 8));

        // Logistic fields
        slab->inuse = 0;
        slab->free = count;


        // Linked list stuff - append function will do the work for us
        slab->next = NULL;
        slab->last = NULL;

        mm_append_slabs(cache->slabs_free, slab);

        slab += sizeof(struct mm_slab);
    }
}

static bool mm_slab_block_is_free(uint8_t* bitmap, uint64_t i) {
    return (bitmap[i / 8] >> (i % 8)) & 1;
}

static void mm_slab_set_block_used(uint8_t* bitmap, uint64_t i) {
    bitmap[i / 8] = bitmap[i / 8] & ~(1 << (i % 8));
}

static uint64_t mm_slab_find_free_object(uint8_t* bitmap, uint64_t starting_addr, uint64_t obj_size, uint64_t objects_to_allocate, uint64_t possible_objects_in_bitmap) {
    if (objects_to_allocate == 0) {
        return __MM_FIND_FREE_OBJECT_FAIL;
    }

    bool valid_block = false;
    for (size_t i = 0; i < possible_objects_in_bitmap * 8; i++) {
        serial_puts("\n[SLAB] mm_slab_find_free_object: Searching block ");
        serial_puts(unsigned_long_to_str(i));
        if (mm_slab_block_is_free(bitmap, i)) {
            serial_puts("\n - [-] Found possible free block...");
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
                serial_puts("\n--------------------");
                return starting_addr + (i * obj_size);
            }
        }
    }

    return __MM_FIND_FREE_OBJECT_FAIL;
}

static int mm_slab_rotate_slabs(struct mm_slab** dstslab, struct mm_slab** srcslab_head, struct mm_slab* srcslab) {
    if(!srcslab || !*srcslab_head)
        return -1;

    if(srcslab->next != NULL)
        srcslab->next->last = srcslab->last;
    if(srcslab->last != NULL)
        srcslab->last->next = srcslab->next;
    if(*srcslab_head == srcslab)
        *srcslab_head = srcslab->next;

    if(!*dstslab) {
        srcslab->last = NULL;
        srcslab->next = NULL;
        *dstslab = srcslab;
        return 0;
    }

    struct mm_slab *node = *dstslab;
    while(node->next != NULL)
        node = node->next;
    node->next = srcslab;
    srcslab->last = node;

    return 0;
}

static void mm_move_free_slabs_to_partial_list(struct mm_slab_cache* cache) {
    mm_slab_rotate_slabs(&(cache->slabs_partial), &(cache->slabs_free), cache->slabs_free);
    mm_alloc_slabs(cache, cache->batchcount);

    serial_puts("\n - slabs_partial: ");
    mm_slab_debug_slabs(cache->slabs_partial);

    serial_puts("\n - slabs_free: ");
    mm_slab_debug_slabs(cache->slabs_free);
}

void* kmalloc(uint64_t size) {
    // Get the cache with suitable-sized objects
    serial_puts("\n[SLAB] KMALLOC STARTING...");
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
        if (slab_start != NULL) {
            //mm_slab_debug_slabs(slab_start);
            uint64_t free_obj = mm_slab_find_free_object(slab_start->bitmap, (uint64_t)slab_start->buff, start->object_size, DIV_ROUND_UP(size, start->object_size), start->batchcount);
            if (free_obj != __MM_FIND_FREE_OBJECT_FAIL) {
                slab_start->free -= DIV_ROUND_UP(size, start->object_size);
                slab_start->inuse += DIV_ROUND_UP(size, start->object_size);
                return (void*)free_obj;
            }
            slab_start = slab_start->next;
        }
    }

    // Otherwise, allocate slabs, move batchcount slabs to slabs_partial and start again
    mm_alloc_slabs(start, start->batchcount);
    mm_move_free_slabs_to_partial_list(start);
    return kmalloc(size);
}

void mm_slab_debug_slabs(struct mm_slab* slab) {
    if (slab == NULL) {
        serial_puts("\n\t - NULL");
        return;
    }

    struct mm_slab* start = slab;
    while (start->next != NULL) {
        /*
        serial_puts("{parent cache: ");
        serial_puts(unsigned_long_to_str((uint64_t)start->parent_cache));
        serial_puts(", buffer address: ");
        serial_puts(unsigned_long_to_str((uint64_t)start->buff));
        serial_puts(", bitmap address: ");
        serial_puts(unsigned_long_to_str((uint64_t)start->bitmap));
        serial_puts(", inuse: ");
        serial_puts(unsigned_long_to_str((uint64_t)start->inuse));
        serial_puts(", free: ");
        serial_puts(unsigned_long_to_str((uint64_t)start->free));
        serial_puts("}");
        serial_puts(" --> ");
        */
        mm_slab_debug_slabs_no_next(start);
        start = start->next;
    }
    mm_slab_debug_slabs_no_next(start);
}

void mm_slab_debug_slabs_no_next(struct mm_slab* slab) {
    if (slab == NULL) {
        serial_puts("\n\t - NULL");
        return;
    }

    serial_puts("\n\t - ");
    struct mm_slab* start = slab;
    serial_puts("__ADDR: ");
    serial_puts(unsigned_long_to_str((uint64_t)slab));
    serial_puts(" ");
    serial_puts("{parent cache: ");
    serial_puts(unsigned_long_to_str((uint64_t)start->parent_cache));
    serial_puts(", buffer address: ");
    serial_puts(unsigned_long_to_str((uint64_t)start->buff));
    serial_puts(", bitmap address: ");
    serial_puts(unsigned_long_to_str((uint64_t)start->bitmap));
    serial_puts(", inuse: ");
    serial_puts(unsigned_long_to_str((uint64_t)start->inuse));
    serial_puts(", free: ");
    serial_puts(unsigned_long_to_str((uint64_t)start->free));
    serial_puts(", next: ");
    serial_puts(unsigned_long_to_str((uint64_t)start->next));
    serial_puts(", last: ");
    serial_puts(unsigned_long_to_str((uint64_t)start->last));
    serial_puts("}");
}

void mm_slab_debug() {
    struct mm_slab_cache* start = root_cache;
    uint64_t i = 0;
    while (start->next != NULL) {
        serial_puts("\n-----------------------");
        serial_puts("\n[+] Cache Name: ");
        serial_puts(start->cache_name);
        serial_puts("\n[+] Flags: ");
        serial_puts(unsigned_long_to_str((uint64_t)start->flags));
        serial_puts("\n[+] Object size: ");
        serial_puts(unsigned_long_to_str(start->object_size));
        serial_puts("\n[+] Limit: ");
        serial_puts(unsigned_long_to_str(start->limit));
        serial_puts("\n[+] Batchount: ");
        serial_puts(unsigned_long_to_str(start->batchcount));
        serial_puts("\n[+] Slab mutex state: ");
        serial_puts(unsigned_long_to_str(start->slab_mu));
        struct mm_slab* starting_slabs_full = start->slabs_full;
        struct mm_slab* starting_slabs_partial = start->slabs_partial;
        struct mm_slab* starting_slabs_free = start->slabs_free;
        serial_puts("\n - SLABS_FULL: ");
        mm_slab_debug_slabs(starting_slabs_full);
        serial_puts("\n - SLABS_PARTIAL: ");
        mm_slab_debug_slabs(starting_slabs_partial);
        serial_puts("\n - SLABS_FREE: ");
        mm_slab_debug_slabs(starting_slabs_free);
        serial_puts("\n-----------------------");
        start = start->next;
    }
}