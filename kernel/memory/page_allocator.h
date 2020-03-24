//
// Created by dxy on 2020/3/20.
//

/**
 * It's a physical page allocator which manages all physical pages allocation and release.
 * It contains all kernel available physical pages and will ensure that no reserved physical memory is modified.
 * Its smallest allocation and release size is a page and all memory requests are dealt with this allocator.
 * It starts working almost immediately after the bootloader completes its work,
 * so some kernel's global data structure, such as GDT(global descriptor table) and PD( kernel'spage directory),
 * will use this allocator to get memory.
 * All memory references in this allocator are physical memory,
 * so the translation of virtual address and modification of page table should be completed by requester themselves.
 */
#ifndef XYOS_PAGE_ALLOCATOR_H
#define XYOS_PAGE_ALLOCATOR_H

#include <stdint.h>

#define PAGE_SIZE 0x1000u
#define VIRTUAL_PAGE_NUM 0x100000u
// minimum physical page number after kernel's 512 MB direct map area
#define DYNAMIC_MAP_START 0x20000u
// kernel's continuous memory maximum: 128 MB
#define K_MAX_CONTINUOUS_PAGE_NUM 0x8000u

// 'byte_size' must be a multiple of 'PAGE_SIZE'
#define byte_to_page(byte_size) byte_size / PAGE_SIZE

#define page_physical_address(n) n * PAGE_SIZE
#define page_number(physical_addr) physical_addr / PAGE_SIZE

/**
 * use a bit map to indicate whether physical page n is allocated
 * 0 indicates free and 1 indicates allocated
 */
uint64_t page_state[0x4000u];

typedef struct available_physical_mem {
    uint32_t addr;
    uint32_t size;
    struct available_physical_mem *next;
} available_physical_mem_t;

/**
 * Store available memory information provided by the bootloader.
 * It is used to detect whether the physical page is valid to allocate.
 */
available_physical_mem_t *available_mem;

/**
 * set all unavailable pages and clear all available pages
 */
void page_allocator_init();

/**
 * allocate an available physical page except those for kernel's direct mapping area
 * @return physical address of the allocated area, if a failure happens, return 0
 */
uint32_t request_page();

/**
 * This function is only for special kernel requests which need to store some global data in fixed physical address,
 * thus it is mostly called in kernel's initialization and only allowed in direct map area.
 * @param n it should be less than 'DYNAMIC_MAP_START'
 * @return physical address of the allocated area, if a failure(request allocated page or reserved page) happens, return -1
 */
int32_t k_request_page(uint32_t n);

/**
 * request continuous 'amount' pages from page 'n'
 * @param n the first requested page number
 * @param amount requested page amounts
 *               Its maximum is MAX_CONTINUOUS_PAGE_NUM, say, kernel can only request continuous memory less than 128 MB.
 * @return Return nth page's physical address of the allocated area in successful allocation.
 *         If any failure happens when requesting any of the 'amount' pages, return -1.
 */
int32_t k_request_pages(uint32_t n,uint16_t amount);

/**
 * set page n to free state and clear all bytes to 0
 * If nth page is not available memory, this function will do nothing.
 * @param n nth physical page
 */
void release_page(uint32_t n);


#endif //XYOS_PAGE_ALLOCATOR_H