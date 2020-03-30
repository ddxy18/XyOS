//
// Created by dxy on 2020/3/28.
//

#include "memory_manager.h"
#include "../driver/console.h"
#include "direct_mapping_allocator.h"
#include "page_allocator.h"
#include "paging.h"
#include "segment.h"

/**
 * @brief Store available memory information provided by the bootloader.
 * It will provide available memory's physical address and size, so page
 * allocator can use it to initialize 'page_state'. Only page allocator and
 * kernel direct mapping allocator will use it to initialize themselves, so it
 * can be overwritten after the two allocators initialized. The last node's size
 * will be set to 0xffffffffu as a sign of end.
 */
avl_phys_mem_t *avl_mem = (avl_phys_mem_t *)AVL_MEM_ADDR;

avl_phys_mem_t *get_avl_mem() { return avl_mem; }

void mem_init() {
  printf("Open paging...\n");
  // Set kernel page directory and direct mapping area's page tables. From now
  // on, addresses we reference are always virtual addresses.
  k_paging_init();
  printf("Reset GDT...\n");
  // move GDT to a safe reserved place
  gdt_init();
  printf("Initialize page allocator...\n");
  page_allocator_init();
  printf("Initialize direct mapping allocator...\n");
  direct_mapping_init();
}