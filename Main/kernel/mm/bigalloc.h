#ifndef BIGALLOC_H
#define BIGALLOC_H

#include <stdint.h>
#include <stddef.h>

struct mm_bigalloc_node {
    uint64_t base;
    uint64_t size;

    struct mm_bigalloc_node* node;
};

static struct mm_bigalloc_node* mm_bigalloc_root;

void mm_bigalloc_init();

#endif // BIGALLOC_H