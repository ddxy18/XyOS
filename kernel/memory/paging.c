//
// Created by dxy on 2020/3/17.
//

/**
 * now only support 4 KB page sizes
 * page cache is disabled
 * page directory locates in 0x0~0x1000
 * kernel
 * page tables must be 4 KB aligned
 */

#include "paging.h"
#include "direct_mapping_allocator.h"
#include "page_allocator.h"

CR3_t cr3;

void set_page_table_entry(page_table_entry_t *, uint8_t, uint32_t);

/**
 * initialize kernel's page directory
 * initialize 512 MB direct mapping area to the physical address space from 0x0
 * initialize remaining 128 MB kernel space reserved for dynamic mapping
 */
void k_paging_init() {
    // request memory for kernel's page directory
    cr3.page_directory_addr = request_fixed_bytes(K_PAGE_DIRECTORY_PHYSICAL_ADDR, PAGE_DIRECTORY_SIZE);
    // TODO: deal with memory allocation failure
    cr3.access = 0;
    cr3.ignored = 0;

    page_directory_entry_t *k_page_directory;
    k_page_directory = (page_directory_entry_t *) cr3.page_directory_addr;

    // set user space's page directory entries to absent state
    uint16_t i = 0;
    for (; i < U_PDE_NUM; ++i) {
        k_page_directory[i].access = U_PDE_WRITE_ABSENT_ACCESS;
        k_page_directory[i].ignored = 0x0u;
        k_page_directory[i].page_table_addr = PDE_ABSENT_ADDR;
    }

    // initialize direct mapping area
    // page_table_offset indicates the current page table's physical address
    uint32_t page_table_offset = request_fixed_bytes(K_PAGE_DIRECTORY_PHYSICAL_ADDR,
                                                     PAGE_TABLE_MAX_SIZE * K_PDE_DIRECT_MAP_NUM);
    uint16_t page_addr = 0x0u;
    for (; i < K_PDE_DIRECT_MAP_NUM + U_PDE_NUM; ++i) {
        k_page_directory[i].access = K_PDE_WRITE_PRESENT_ACCESS;
        k_page_directory[i].ignored = 0x0u;
        k_page_directory[i].page_table_addr = pde_addr(page_table_offset);

        page_table_entry_t *page_table = (page_table_entry_t *) page_table_offset;
        for (uint16_t j = 0; j < PTE_MAX_NUM; ++j) {
            page_table[j].access = K_PTE_WRITE_PRESENT_ACCESS;
            page_table[j].ignored = 0x0u;
            page_table[j].page_addr = pte_addr(page_addr);
            page_addr += PAGE_SIZE;
        }
        page_table_offset += PAGE_TABLE_MAX_SIZE;
    }
    // set all dynamic mapping page directory entries to absent state
    for (; i < PDE_NUM; ++i) {
        k_page_directory[i].access = K_PDE_WRITE_ABSENT_ACCESS;
        k_page_directory[i].ignored = 0x0u;
        k_page_directory[i].page_table_addr = PDE_ABSENT_ADDR;
    }

    asm volatile ("movl %0, cr3"
    :
    :"m" (cr3)
    );
    // set CR0.PG = 1 and CR4.PAE = 0 to open 32-bit paging
    uint32_t cr0;
    asm volatile ("movl cr0, %0"
    :"=m" (cr0)
    );
    cr0 |= 0x80000000u;
    asm volatile ("movl %0, cr0"
    :
    :"m" (cr0)
    );
    uint32_t cr4;
    asm volatile ("movl cr4, %0"
    :"=m" (cr4)
    );
    cr4 |= 0x00000010u;
    asm volatile ("movl %0, cr4"
    :
    :"m" (cr4)
    );
}

void set_page_table_entry(page_table_entry_t *entry, uint8_t access, uint32_t address) {
    entry->access = access;
    entry->ignored = 0x0u;
    entry->page_addr = address;
}