#ifndef BIGALLOC_H
#define BIGALLOC_H

#include <stdint.h>
#include <stddef.h>
#include "../sched/mutex.h"
#include "pmm.h"
#include "vmm.h"

static uint8_t mm_bigalloc_mu;

struct mm_bigalloc_node {
    uint64_t base;
    uint64_t size;

    struct mm_bigalloc_node* next;
};

static struct mm_bigalloc_node* mm_bigalloc_root;

void mm_bigalloc_init();

void* mm_bigalloc_alloc(uint64_t size);
void mm_bigalloc_free(void* ptr);

static void mm_bigalloc_append(struct mm_bigalloc_node* add);

#endif // BIGALLOC_H