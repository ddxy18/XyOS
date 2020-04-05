//
// Created by dxy on 2020/3/17.
//

/**
 * use 32-bit paging policy
 */

#ifndef XYOS_PAGING_H
#define XYOS_PAGING_H

#include <stdint.h>
#include "../interrupt/idt.h"

// kernel will use 3GB through 4GB linear address space
#define K_VIRTUAL_ADDR 0xc0000000u
#define K_VIRTUAL_SIZE 0x80000000u

// page directory's properties
#define K_PD_ADDR 0xc0001000u
#define PDE_NUM 1024u
#define U_PDE_NUM 768u
#define K_PDE_NUM 256u
#define K_PDE_DIRECT_MAP_NUM 16u
#define PDE_SIZE 4u
#define PD_SIZE PDE_NUM *PDE_SIZE
#define PDE_ABSENT_ADDR 0x0u

/**
 * @def pd_addr(phys_addr)
 *
 * CR3 only reserve 20 bits to store page directory's physical address,
 * so page directory will be 4 KB aligned and CR3 only store the upper 20 bits.
 */
#define pd_addr(phys_addr) ((phys_addr) >> 12u)
/**
 * @def pt_addr(phys_addr)
 *
 * Only reserve 20 bits to store physical table's physical address,
 * so page table will be 4 KB aligned and PDE only store the upper 20 bits.
 */
#define pt_addr(phys_addr) ((phys_addr) >> 12u)
/**
 * @def page_addr(phys_addr)
 *
 * PDE only store the upper 20 bits of the physical page's physical address.
 */
#define page_addr(phys_addr)((phys_addr) >> 12u)

// page table's global properties
// page tables max size in bytes
#define PT_MAX_SIZE PTE_MAX_NUM *PTE_SIZE
#define K_PT_ADDR 0xc000c000u
#define PTE_MAX_NUM 1024u
#define PTE_SIZE 4u
#define PTE_ABSENT_ADDR 0x0u

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

/**
 * @typedef pde_t page directory entry
 *
 * Page directory must be 4 KB aligned.
 */
typedef struct pde {
    uint8_t access;
    uint8_t ignored: 4;
    // store high 20 bits because page tables are 4 KB aligned
    uint32_t page_table_addr: 20;
} __attribute__((packed)) pde_t;

/**
 * @typedef pte_t page table entry
 *
 * Page table must be 4 KB aligned.
 */
typedef struct pte {
    uint16_t access: 9;
    uint8_t ignored: 3;
    uint32_t page_addr: 20;
} __attribute__((packed)) pte_t;

typedef struct CR3 {
    // 2:0 in CR3 is ignored
    uint8_t access: 5;
    uint8_t ignored: 7;
    uint32_t page_dir_addr: 20;
} CR3_t;

/**
 * Initialize kernel's page directory. Initialize 64 MB direct mapping
 * area to the physical address space from 0x0. Set remaining 960 MB kernel
 * space reserved for dynamic mapping.
 */
void k_paging_init();

/**
 * Creating a new process will use this function to set its page directory.
 * It will set all user space entry to absent state and copy kernel
 * space entry in corresponding location. Caller should request memory for page
 * directory itself.
 *
 * @param page_dir
 */
void u_page_dir_init(pde_t *page_dir);

/**
 * Release all pages exist in 'page_table'. Notice that 'page_table' itself should be released by caller itself.
 *
 * @param page_table
 */
void u_release_pages(pte_t *page_table);

/**
 * @brief page fault handler
 *
 * @verbatim
 * Now there are only 3 possible sources of PF exception.
 * 1. page not present
 * 2. invalid mode:
 *      - kernel access address below than 0xc0000000u
 *      - user access address above than 0xc0000000u
 * 3. write to a read-only page
 * @endverbatim
 */
void pf_handler(intr_reg_t *intr_reg);

#endif // XYOS_PAGING_H