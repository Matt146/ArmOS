#ifndef SLAB_H
#define SLAB_H

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include "pmm.h"
#include "vmm.h"
#include "../driver/serial/serial.h"
#include "../sched/mutex.h"

#define SLAB_INITIAL_ALLOC 5
#define SLAB_STD_BATCHCOUNT 5
#define __NONE 0

#define DIV_ROUND_UP(n, d) (((n) + (d) - 1) / (d))

#define __MM_FIND_FREE_OBJECT_FAIL PMM_ALLOC_FAIL

// Slab flags
#define SLAB_RED_ZONE (1 << 2)          // Traps overflows for debug purposes
#define SLAB_POISON (1 << 3)            // Poisons objects with known pattern for debug purposes

struct mm_slab_cache {
    // General fields
    const char* cache_name;
    struct mm_slab* slabs_full;
    struct mm_slab* slabs_partial;
    struct mm_slab* slabs_free;
    uint32_t flags;
    uint64_t object_size;

    // SMP added fields
    uint64_t limit;         // limit of free object the slabs_partial can have before its slabs are given to slabs_free
    uint64_t batchcount;
    uint8_t slab_mu;        // mutex

    // Linked list stuff
    struct mm_slab_cache* next;
    struct mm_slab_cache* last;
} __attribute__((packed));

struct mm_slab {
    // Memory fields
    struct mm_slab_cache* parent_cache;
    void* buff;
    uint8_t* bitmap;    // each bit in the bitmap represents object_size bytes

    // Logistic fields - UNUSED: IMPLEMENT LATER
    uint64_t inuse;     //  Number of in-use objects in slab
    uint64_t total_available;      // Number of total objects in the slab

    // Linked list stuff
    struct mm_slab* next;
    struct mm_slab* last;
} __attribute__((packed));

static struct mm_slab_cache* root_cache;   // The head :dababy:

void mm_slab_init();
struct mm_slab_cache* mm_slab_create_cache(const char* name, uint32_t flags, uint64_t object_size);
void* kmalloc(uint64_t size);
void mm_slab_debug();
void mm_slab_debug_slabs(struct mm_slab* slab);
void mm_slab_debug_slabs_no_next(struct mm_slab* slab);

static void mm_alloc_slabs(struct mm_slab_cache* cache, uint64_t count);
static void mm_append_slabs(struct mm_slab* dstslab, struct mm_slab* srcslab);
static uint64_t mm_slab_find_free_object(uint8_t* bitmap, uint64_t starting_addr, uint64_t obj_size, uint64_t objects_to_allocate, uint64_t possible_objects_in_bitmap);

static bool mm_slab_block_is_free(uint8_t* bitmap, uint64_t i);                                     // Sets a block free in a bitmap
static void mm_slab_set_block_used(uint8_t* bitmap, uint64_t i);                                    // Sets a block used in a bitmap
static void mm_move_free_slabs_to_partial_list(struct mm_slab_cache* cache);          // Moves num slab_frees to slabs_partial list
static int mm_slab_rotate_slabs(struct mm_slab** dstslab, struct mm_slab** srcslab_head, struct mm_slab* srcslab);

#endif // SLAB_H