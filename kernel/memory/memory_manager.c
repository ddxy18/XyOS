//
// Created by dxy on 2020/3/28.
//

#include "memory_manager.h"

#include "../driver/console.h"
#include "direct_mapping_allocator.h"
#include "segment.h"

const int kAvlMemAddr = 0x20000u;

/**
 * @brief Store available memory information provided by the bootloader.
 *
 * It provides available memory's physical address and size, so page
 * allocator can use it to initialize 'page_state'. Only page allocator uses
 * it to initialize themselves, so it can be overwritten after page allocator
 * initialized. The last node's size will be set to 0xffffffffu as a sign of
 * end.
 */
static AvlPhysMem *const avl_mem = (AvlPhysMem *) kAvlMemAddr;

AvlPhysMem *GetAvlMem() { return avl_mem; }

void MemInit() {
    printf("Open paging...\n");
    // Set kernel page directory and direct mapping area's page tables. From now
    // on, addresses we reference are always virtual addresses.
    PagingInit();
    printf("Reset GDT...\n");
    // move GDT to page 0
    GdtInit();
    printf("Initialize direct mapping allocator...\n");
    DirectMappingInit();
}