//
// Created by dxy on 2020/3/17.
//

/**
 * use 32-bit paging policy
 */

#ifndef XYOS_PAGING_H
#define XYOS_PAGING_H

#include <stdint.h>

// kernel will use 3GB through 4GB linear address space
#define K_VIRTUAL_ADDR 0xc0000000u
#define K_VIRTUAL_SIZE 0x80000000u

// page directory's properties
#define K_PAGE_DIRECTORY_PHYSICAL_ADDR 0x0u
#define PDE_NUM 1024u
#define U_PDE_NUM 768u
#define K_PDE_NUM 256u
#define K_PDE_DIRECT_MAP_NUM 128u
#define PDE_SIZE 4u
#define PAGE_DIRECTORY_SIZE PDE_NUM * PDE_SIZE
#define PDE_ABSENT_ADDR 0x0u
/**
 * PDE only reserve 20 bits to store page table's physical address,
 * so page table will be 4 KB aligned and PDE only store the upper 20 bits
 */
#define pde_addr(physical_addr) physical_addr>>12u

// page table's global properties
// page tables max size in bytes
#define PAGE_TABLE_MAX_SIZE PTE_MAX_NUM * PTE_SIZE
#define K_PAGE_TABLE_ADDR 0x100000u
#define PTE_MAX_NUM 1024u
#define PTE_SIZE 4u
#define PTE_ABSENT_ADDR 0x0u
/**
 * PTE only reserve 20 bits to store physical page's physical address,
 * so page table will be 4 KB aligned and PDE only store the upper 20 bits
 */
#define pte_addr(physical_addr) physical_addr>>12u

// PDE and PTE access options, cache policy is disabled
// PDE
#define K_PDE_READ_PRESENT_ACCESS 0x1u
#define K_PDE_WRITE_PRESENT_ACCESS 0x3u
#define K_PDE_READ_ABSENT_ACCESS 0x0u
#define K_PDE_WRITE_ABSENT_ACCESS 0x2u
#define U_PDE_READ_PRESENT_ACCESS 0x5u
#define U_PDE_WRITE_PRESENT_ACCESS 0x7u
#define U_PDE_READ_ABSENT_ACCESS 0x4u
#define U_PDE_WRITE_ABSENT_ACCESS 0x6u
// PTE
#define K_PTE_READ_PRESENT_ACCESS 0x1u
#define K_PTE_WRITE_PRESENT_ACCESS 0x3u
#define K_PTE_READ_ABSENT_ACCESS 0x0u
#define K_PTE_WRITE_ABSENT_ACCESS 0x2u
#define U_PTE_READ_PRESENT_ACCESS 0x5u
#define U_PTE_WRITE_PRESENT_ACCESS 0x7u
#define U_PTE_READ_ABSENT_ACCESS 0x4u
#define U_PTE_WRITE_ABSENT_ACCESS 0x6u

typedef struct page_directory_entry {
    uint8_t access;
    uint8_t ignored: 4;
    // store high 20 bits because page tables are 4 KB aligned
    uint32_t page_table_addr: 20;
} page_directory_entry_t;

typedef struct page_table_entry {
    uint16_t access: 9;
    uint8_t ignored: 3;
    uint32_t page_addr: 20;
} page_table_entry_t;

typedef struct CR3 {
    // 2:0 in CR3 is ignored
    uint8_t access: 5;
    uint8_t ignored: 7;
    uint32_t page_directory_addr: 20;
} CR3_t;

void k_paging_init();

/**
 * Creating a new process will use this function to set its page directory.
 * Caller should request memory for page directory itself.
 * @param page_directory
 */
void u_page_directory_init(page_directory_entry_t *page_directory);

#endif //XYOS_PAGING_H