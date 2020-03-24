//
// Created by dxy on 2020/3/20.
//

#include <stddef.h>

#include "page_allocator.h"
#include "../lib/bitmap.h"
#include "paging.h"

void clear_page(uint32_t n);

void page_allocator_init() {
    available_physical_mem_t *t = available_mem;
    uint32_t addr = 0;
    while (t->size != -1) {
        // set all unavailable page's bits
        while (addr < t->addr) {
            set_bit(page_state, addr / PAGE_SIZE);
            addr += PAGE_SIZE;
        }
        // clear all available page's bits
        while (addr + PAGE_SIZE <= t->addr + t->size) {
            clear_bit(page_state, addr / PAGE_SIZE);
            addr += PAGE_SIZE;
        }
        t = t->next;
    }
}

uint32_t request_page() {
    for (uint32_t i = DYNAMIC_MAP_START; i < VIRTUAL_PAGE_NUM; ++i) {
        if (!is_bit_set(page_state, i)) {
            set_bit(page_state, i);
            return page_physical_address(i);
        }
    }
    return 0;
}

int32_t k_request_page(uint32_t n) {
    // requested page out of boundary
    if (n > DYNAMIC_MAP_START) {
        return -1;
    }
    // check if requested page has already been allocated or is reserved to prevent dangerous overwriting
    if (!is_bit_set(page_state, n)) {
        set_bit(page_state, n);
        return page_physical_address(n);
    }
    return -1;
}

int32_t k_request_pages(uint32_t n, uint16_t amount) {
    // memory request out of system bound
    if (amount > K_MAX_CONTINUOUS_PAGE_NUM) {
        return -1;
    }

    for (uint16_t i = 0; i < amount; ++i) {
        if (k_request_page(i + n) < 0) {
            // release pages have been allocated
            for (uint16_t j = 0; j < i; ++j) {
                release_page(j + n);
            }
            return -1;
        }
    }
    return page_physical_address(n);
}

void release_page(uint32_t n) {
    uint32_t addr = page_physical_address(n);
    available_physical_mem_t *t = available_mem;
    while (t->size != -1) {
        if (t->addr < addr && t->addr + t->size > addr + PAGE_SIZE) {
            clear_page(n);
            clear_bit(page_state, n);
            return;
        }
        t = t->next;
    }
    t = NULL;
}

/**
 * set all bytes in this page to 0
 * @param n nth physical page
 */
void clear_page(uint32_t n) {
    uint32_t physical_addr = page_physical_address(n);
    uint32_t *byte_pointer = (uint32_t *) physical_addr;
    for (; (uint32_t) byte_pointer < physical_addr + PAGE_SIZE; ++byte_pointer) {
        *byte_pointer = 0;
    }
}