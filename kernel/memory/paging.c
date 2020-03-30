//
// Created by dxy on 2020/3/17.
//

/**
 * now only support 4 KB page sizes
 * page cache is disabled
 * page dir locates in 0x1000~0x1fff
 * kernel page tables must be 4 KB aligned
 */

#include "paging.h"
#include "../driver/console.h"
#include "direct_mapping_allocator.h"
#include "page_allocator.h"

CR3_t cr3;

void set_page_table_entry(page_table_entry_t *, uint8_t, uint32_t);

void k_paging_init() {
  cr3.page_dir_addr = pde_addr(K_PAGE_DIR_PHYSICAL_ADDR);
  cr3.access = 0;
  cr3.ignored = 0;

  page_dir_entry_t *k_page_dir = (page_dir_entry_t *)(cr3.page_dir_addr << 12u);

  pointer_t pt_addr = K_PAGE_TABLE_ADDR;
  // Map first 4 MB memory to the same physical address. So when opening paging,
  // current address can also be used.
  k_page_dir[0].access = K_PDE_WRITE_PRESENT_ACCESS;
  k_page_dir[0].ignored = 0x0u;
  k_page_dir[0].page_table_addr = pde_addr(pt_addr);
  page_table_entry_t *page_table = (page_dir_entry_t *)pt_addr;
  for (uint32_t j = 0; j < PTE_MAX_NUM; ++j) {
    page_table[j].access = K_PTE_WRITE_PRESENT_ACCESS;
    page_table[j].ignored = 0x0u;
    page_table[j].page_addr = pte_addr(j * PAGE_SIZE);
  }
  pt_addr += PAGE_TABLE_MAX_SIZE;
  // set remaining user space's page dir entries to absent state
  uint32_t i = 1;
  for (; i < U_PDE_NUM; ++i) {
    k_page_dir[i].access = U_PDE_WRITE_ABSENT_ACCESS;
    k_page_dir[i].ignored = 0x0u;
    k_page_dir[i].page_table_addr = PDE_ABSENT_ADDR;
  }
  // initialize direct mapping area
  pointer_t page_addr = 0x0u;
  for (; i < K_PDE_DIRECT_MAP_NUM + U_PDE_NUM; ++i) {
    k_page_dir[i].access = K_PDE_WRITE_PRESENT_ACCESS;
    k_page_dir[i].ignored = 0x0u;
    k_page_dir[i].page_table_addr = pde_addr(pt_addr);

    page_table_entry_t *page_table = (page_table_entry_t *)pt_addr;
    for (uint32_t j = 0; j < PTE_MAX_NUM; ++j) {
      page_table[j].access = K_PTE_WRITE_PRESENT_ACCESS;
      page_table[j].ignored = 0x0u;
      page_table[j].page_addr = pte_addr(page_addr);
      page_addr += PAGE_SIZE;
    }
    pt_addr += PAGE_TABLE_MAX_SIZE;
  }
  // set all dynamic mapping area's page dir entries to absent state
  for (; i < PDE_NUM; ++i) {
    k_page_dir[i].access = K_PDE_WRITE_ABSENT_ACCESS;
    k_page_dir[i].ignored = 0x0u;
    k_page_dir[i].page_table_addr = PDE_ABSENT_ADDR;
  }

  // let '%cr3' point to kernel's page dir
  asm("mov %0, %%eax" ::"r"(cr3.page_dir_addr << 12u));
  asm("mov %eax, %cr3");
  // set CR0.PG = 1 and CR4.PAE = 0 to open 32-bit paging
  asm("mov %cr4, %eax");
  asm("or $0x80000010, %eax");
  asm("mov %eax, %cr4");
  asm("mov %cr0, %eax");
  asm("or $0x80000000, %eax");
  asm("mov %eax, %cr0");
  // set '%esp' to its corresponding virtual address
  asm("add $0xc0000000u, %esp");
  // TODO: update all registers containing addresses to virtual addresses
}

void set_page_table_entry(page_table_entry_t *entry, uint8_t access,
                          uint32_t address) {
  entry->access = access;
  entry->ignored = 0x0u;
  entry->page_addr = address;
}

void u_page_dir_init(page_dir_entry_t *page_dir) {
  uint32_t i = 0;
  // set user space
  for (; i < U_PDE_NUM; ++i) {
    page_dir[i].ignored = 0x0u;
    page_dir[i].access = U_PDE_WRITE_ABSENT_ACCESS;
    page_dir[i].page_table_addr = PTE_ABSENT_ADDR;
  }
  // copy kernel space's page dir entry
  for (; i < PDE_NUM; ++i) {
    page_dir_entry_t *k_page_dir = (page_dir_entry_t *)K_PAGE_DIR_PHYSICAL_ADDR;
    page_dir[i].ignored = k_page_dir[i].ignored;
    page_dir[i].access = k_page_dir[i].access;
    page_dir[i].page_table_addr = k_page_dir[i].page_table_addr;
  }
}

void u_release_pages(page_table_entry_t *page_table) {
  for (uint32_t i = 0; i < PTE_MAX_NUM; ++i) {
    if (page_table[i].page_addr != PTE_ABSENT_ADDR) {
      release_page(page_no(page_table[i].page_addr));
    }
  }
}