#ifndef VMM_H
#define VMM_H

#include <stdint.h>
#include "pmm.h"
#include "../driver/vga/vga.h"

#define VMM_PAGE_TABLES_START 0x100000  // This is the start of the address space that we can use for paging structures
#define VMM_PAGE_TABLES_END BITMAP_START_ADDR    // This is the end of the address space that we can use for paging structures

/*
 *PML4 TABLE 2MB:
 *  - 63: NX: Set to 0 if code can be executed in this page. Set to 1 if you don't want code to be executed. NOTE: MUST SET EFER.NXE to 1
 * - 62..52: AVAILABLE
 * - 51..12: PDPT base
 * - 11..9: AVL
 * - 8..7: MBZ
 * - 6: IGN: Ignored. Set to 0
 * - 5: A: Set to 1 by the processor the first time the table or physical page is either read from or written to. Never cleared by the processor. Software must clear bit
 * - 4: PCD: Cleared means the table or physical page is cacheable. When set to 1, the table or physical page is NOT cacheable.
 * - 3: PWT: Indicates whether the page-translation table or physical page to which the entry points has a writeback o writethrough policy. Cleared means the table or physical page has a writeback caching policy. 1 means the table or physical page has a writethrough caching policy
 * - 2: U/S: When cleared, restricted to CPL 0, 1, and 2. Set to 1, ring 3 can access it too
 * - 1: R/W: set to 1 if both read and write access are allowed
 * - 0: P: set to 1 if table or physical page is loaded in physical memory
*/
void map_vaddr_to_paddr(uint64_t paddr, uint64_t vaddr, uint8_t attributes);

#endif // VMM_H
