#include "bigalloc.h"

void mm_bigalloc_init() {
    mm_bigalloc_root = pmm_alloc(1) + GLOBAL_OFFSET;
    mm_bigalloc_root->base = 0;
    mm_bigalloc_root->size = 0;
    mm_bigalloc_root->next = NULL;
}

void* mm_bigalloc_alloc(uint64_t size) {
    struct mm_bigalloc_node* new_node = pmm_alloc(1) + GLOBAL_OFFSET;
    new_node->base = pmm_alloc(DIV_ROUND_UP(size, PMM_PAGE_SIZE)) + GLOBAL_OFFSET;
    new_node->size = size;
    mm_bigalloc_append(new_node);

    return (void*)(new_node->base);
}

void mm_bigalloc_free(void* ptr) {
    serial_puts("\n[bigalloc] INITIALIZING FREE... 1 - ");
    uint64_t ptr_value = (uint64_t)ptr;
    serial_puts(unsigned_long_to_str(ptr_value));

    // Sanity check to see if they want us to delete the head
    if (mm_bigalloc_root->next == NULL) {
        return;
    }
    serial_puts("\n[bigalloc] INITIALIZING FREE... 2");
    struct mm_bigalloc_node* prev = mm_bigalloc_root;
    struct mm_bigalloc_node* head = mm_bigalloc_root->next;

    serial_puts("\n[bigalloc] INITIALIZING FREE... 3 ");
    serial_puts("\n - head->base = ");
    serial_puts(unsigned_long_to_str(head->base));
    serial_puts("\n - head->size = ");
    serial_puts(unsigned_long_to_str(head->size));
    if (head->base <= ptr_value && (head->base + head->size <= ptr_value)) {
        serial_puts("\n[INFO] Freeing... ");
        serial_puts(unsigned_long_to_str(head->base));
        pmm_free(vmm_vaddr_to_paddr(head->base), DIV_ROUND_UP(head->size, PMM_PAGE_SIZE) - 1);
        prev->next = head->next;
        return;
    }

    // Otherwise, proceed
    while (head->next != NULL) {
        serial_puts("\n[bigalloc] INITIALIZING FREE... 3 ");
        serial_puts("\n - head->base = ");
        serial_puts(unsigned_long_to_str(head->base));
        serial_puts("\n - head->size = ");
        serial_puts(unsigned_long_to_str(head->size));
        if (head->base <= ptr_value && (head->base + head->size <= ptr_value)) {
            serial_puts("\n[INFO] Freeing... ");
            serial_puts(unsigned_long_to_str(head->base));
            pmm_free(vmm_vaddr_to_paddr(head->base), DIV_ROUND_UP(head->size, PMM_PAGE_SIZE) - 1);
            prev->next = head->next;
            return;
        }
        head = head->next;
        prev = prev->next;
    }
}

static void mm_bigalloc_append(struct mm_bigalloc_node* add) {
    struct mm_bigalloc_node* dstnode = mm_bigalloc_root;
    while (true) {
        serial_puts("\n[bigalloc] AHHHHHH!");
        if (dstnode->next == NULL) {
            serial_puts("\n[bigalloc] APPENDING... 1");
            add->next = NULL;
            serial_puts("\n[bigalloc] APPENDING... 2");
            dstnode->next = add;
            serial_puts("\n[bigalloc] APPENDING... 3");
            break;
        }
        dstnode = dstnode->next;
    }
}

