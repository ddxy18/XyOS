//
// Created by dxy on 2020/3/20.
//

#include <stddef.h>

#include "../driver/console.h"
#include "../lib/bitmap.h"
#include "memory_manager.h"
#include "page_allocator.h"

void clear_page(uint32_t);

uintptr_t k_request_page(uint32_t);

/**
 * @brief Use a bit map to indicate whether physical page n is allocated. 0
 * indicates free and 1 indicates allocated.
 */
uint64_t page_state[0x4000u];

uint8_t page_allocator_init() {
  avl_phys_mem_t *t = get_avl_mem();

  if (t->size == -1) {
    printf("No available memory for kernel, some error may happen in "
           "bootloader.\nXyOS will stop now.\n");
    return 1;
  }

    uintptr_t addr = 0;
  while (t->size != UINT32_MAX) {
    // set all pages between two available memory slices as unavailable pages
    while (addr < t->addr) {
        bitmap_set(page_state, page_no(addr));
        addr += PAGE_SIZE;
    }
    // set all pages completely in an available memory slice as available pages
    while (addr + PAGE_SIZE <= t->addr + t->size) {
        bitmap_clear(page_state, page_no(addr));
        addr += PAGE_SIZE;
    }
    t++;
  }

  t = NULL;
  // Set all pages haven't referenced until now as unavailable pages.
  // After checking the last page, 'addr += PAGE_SIZE' will overflow to 0, so we
  // will use 0 as a sign of the end.
  while (page_no(addr) != 0) {
      bitmap_clear(page_state, page_no(addr));
      addr += PAGE_SIZE;
  }

  // set first 64 MB as available, so we can allocate them to direct mapping
  // allocator to manage them.
  for (uint32_t i = 0; i < DYNAMIC_MAPPING_START_PAGE; i++) {
      bitmap_clear(page_state, i);
  }

  return 0;
}

uintptr_t request_page() {
    for (uint32_t i = DYNAMIC_MAPPING_START_PAGE; i < VIRTUAL_PAGE_NUM; ++i) {
        if (!is_bitmap_set(page_state, i)) {
            bitmap_set(page_state, i);
            return page_phys_addr(i);
        }
    }
    return 0;
}

/**
 * @brief Request page 'n' in kernel direct mapping area.
 *
 * @param n it should be less than 'DYNAMIC_MAP_START_PAGE'
 *
 * @return uintptr_t Return physical address of the allocated page. if a
 * failure(request allocated page or reserved page) happens, return -1.
 */
uintptr_t k_request_page(uint32_t n) {
    // requested page out of boundary
    if (n >= DYNAMIC_MAPPING_START_PAGE) {
        return -1;
    }
    // check if requested page has already been allocated or is reserved to
    // prevent dangerous overwriting
    if (is_bitmap_set(page_state, n) == 0) {
        bitmap_set(page_state, n);
        return page_phys_addr(n);
    }
    return -1;
}

uintptr_t k_request_pages(uint32_t n, uint16_t amount) {
    // memory request out of system bound
    if (amount > DYNAMIC_MAPPING_START_PAGE) {
        return -1;
    }

    for (uint16_t i = 0; i < amount; ++i) {
        if (k_request_page(i + n) == -1) {
            // page 'i + n' has been allocated, release all pages allocated now
            for (uint16_t j = 0; j < i; ++j) {
                release_page(j + n);
      }
      return -1;
    }
  }

  return page_phys_addr(n);
}

void release_page(uint32_t n) {
    uintptr_t addr = page_phys_addr(n);
    avl_phys_mem_t *t = get_avl_mem();
  while (t->size != -1) {
    if (t->addr < addr && t->addr + t->size >= addr + PAGE_SIZE) {
        clear_page(n);
        bitmap_clear(page_state, n);
        t = NULL;
      return;
    }
    t++;
  }
  t = NULL;
}

/**
 * @brief set all bytes in this page to 0
 *
 * @param n nth physical page
 */
void clear_page(uint32_t n) {
    uintptr_t physical_addr = page_phys_addr(n);
    uintptr_t *byte_pointer = (uintptr_t *) physical_addr;
    for (; (uintptr_t) byte_pointer < physical_addr + PAGE_SIZE; ++byte_pointer) {
        *byte_pointer = 0;
    }
}