//
// Created by dxy on 2020/3/20.
//

/**
 * Page allocator is a physical page allocator which manages all available
 * physical pages' allocation and release. It's a public basic memory allocator
 * and all other memory managers will use it to complete their function.
 *
 * In 'start.c', 'available_mem' will be initialized, so we will get all
 * available memory's physical address and size. We will use 'page_state' to
 * record whether physical page n is allocated.
 *
 * It provides two allocate policy: 'request_page()' will allocate a page and
 * its physical address is random, 'k_request_page(uint32_t)' is for kernel
 * direct mapping memory and will allocate pages in given address. User space
 * and kernel dynamic mapping area will request memory from the allocator
 * through page faults and kernel direct mapping area will request all memory
 * firstly and manage it itself. Moreover, bootloader will open segment and make
 * logical address equal linear address and physical address. So all memory
 * addresses in this allocator are physical addresses. Translation to
 * virtual address and modification of page tables should be completed by
 * requester themselves.
 */
#ifndef XYOS_PAGE_ALLOCATOR_H
#define XYOS_PAGE_ALLOCATOR_H

#include <stdint.h>

#include "../lib/def.h"

// now only support 4 KB pages
#define PAGE_SIZE 0x1000u
// 4 GB memory addresses contain 2^20 4 KB pages
#define VIRTUAL_PAGE_NUM 0x100000u
// first physical page after kernel's 64 MB direct mapping area
#define DYNAMIC_MAPPING_START_PAGE 0x4000u
// restrict a maximum of 64 MB continuous physical memory in kernel dynamic
// area
#define K_MAX_CONTINUOUS_PAGE_NUM 0x4000u

/**
 * translate memory sizes represented in bytes to 4 KB page amounts
 * 'byte_size' must be a multiple of 'PAGE_SIZE'
 */
#define byte_to_page(byte_size) (byte_size / PAGE_SIZE)
// calculate page n's physical address
#define page_phys_addr(n) (n * PAGE_SIZE)
// calculate which page 'physical_addr' locates in
#define page_no(phys_addr) (phys_addr / PAGE_SIZE)

/**
 * @brief It will set 'page_state' according to 'available_mem'. If any bytes in
 * a page is unavailable, this page will be viewed as an unavailable page. All
 * unavailable pages' bits will be set to 1 and others set to 0.
 *
 * @return uint8_t Return 0 in normal. Return 1 if no available memory exists.
 */
uint8_t page_allocator_init();

/**
 * @brief It will search 'page_state' from 'DYNAMIC_MAPPING_START_PAGE' in
 * sequence until find the first free page and then return the page's physical
 * address.
 *
 * @return pointer_t physical address of the allocated page. If a failure
 * happens, return 0. Notice that it will not search reserved area for kernel
 * direct mapping, so the situation that choose page 0 as a free page will never
 * happen.
 */
pointer_t request_page();

/**
 * @brief This function is only used for allocate kernel direct mapping area.
 * Direct mapping allocator will request continuous 'amount' pages starting from
 * page 'n' at first time and manage it itself.
 *
 * @param n the first requested page number
 * @param amount requested page amounts. Its maximum is MAX_CONTINUOUS_PAGE_NUM,
 * say, kernel can only request continuous memory less than 64 MB.
 *
 * @return pointer_t Return nth page's physical address of the allocated area in
 * successful allocation. If any failure happens when requesting any of the
 * 'amount' pages, return -1.
 */
pointer_t k_request_pages(uint32_t n, uint16_t amount);

/**
 * @brief Set page 'n' to free state and clear all bytes to 0.
 * If page 'n' is not available or in free state, this function will do nothing.
 *
 * @param n nth physical page
 */
void release_page(uint32_t n);

#endif // XYOS_PAGE_ALLOCATOR_H