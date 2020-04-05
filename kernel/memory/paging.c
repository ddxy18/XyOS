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
#include "page_allocator.h"
#include "../lib/asm_wrapper.h"
#include "../lib/bit.h"
#include "direct_mapping_allocator.h"
#include "../process/process.h"

CR3_t k_cr3;

void set_page_table_entry(pte_t *, uint8_t, uint32_t);

void k_paging_init() {
    k_cr3.page_dir_addr = pd_addr(direct_mapping_phys_addr(K_PD_ADDR));
    k_cr3.access = 0;
    k_cr3.ignored = 0;

    pde_t *k_page_dir = (pde_t *) direct_mapping_phys_addr(K_PD_ADDR);

    uintptr_t pt_addr = direct_mapping_phys_addr(K_PT_ADDR);
    // Map first 4 MB memory to the same physical address. So when opening paging,
    // current address can also be used.
    k_page_dir[0].access = K_PDE_WRITE_PRESENT_ACCESS;
    k_page_dir[0].ignored = 0x0u;
    k_page_dir[0].page_table_addr = pt_addr(pt_addr);
    pte_t *page_table = (pte_t *) pt_addr;
    for (uint32_t j = 0; j < PTE_MAX_NUM; ++j) {
        page_table[j].access = K_PTE_WRITE_PRESENT_ACCESS;
        page_table[j].ignored = 0x0u;
        page_table[j].page_addr = page_addr(j * PAGE_SIZE);
    }
    pt_addr += PT_MAX_SIZE;
    // set remaining user space's page dir entries to absent state
    uint32_t i = 1;
    for (; i < U_PDE_NUM; ++i) {
        k_page_dir[i].access = U_PDE_WRITE_ABSENT_ACCESS;
        k_page_dir[i].ignored = 0x0u;
        k_page_dir[i].page_table_addr = PDE_ABSENT_ADDR;
    }
    // initialize direct mapping area
    uintptr_t page_addr = 0x0u;
    for (; i < K_PDE_DIRECT_MAP_NUM + U_PDE_NUM; ++i) {
        k_page_dir[i].access = K_PDE_WRITE_PRESENT_ACCESS;
        k_page_dir[i].ignored = 0x0u;
        k_page_dir[i].page_table_addr = pt_addr(pt_addr);

        page_table = (pte_t *) pt_addr;
        for (uint32_t j = 0; j < PTE_MAX_NUM; ++j) {
            page_table[j].access = K_PTE_WRITE_PRESENT_ACCESS;
            page_table[j].ignored = 0x0u;
            page_table[j].page_addr = page_addr(page_addr);
            page_addr += PAGE_SIZE;
        }
        pt_addr += PT_MAX_SIZE;
    }
    // set all dynamic mapping area's page dir entries to absent state
    for (; i < PDE_NUM; ++i) {
        k_page_dir[i].access = K_PDE_WRITE_ABSENT_ACCESS;
        k_page_dir[i].ignored = 0x0u;
        k_page_dir[i].page_table_addr = PDE_ABSENT_ADDR;
    }

    // let '%cr3' point to kernel's page directory
    asm("mov %0, %%eax"::"r"(k_cr3.page_dir_addr << 12u));
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
    // TODO: Update all registers containing addresses to virtual addresses and remove first 4 MB mapping.
}

void set_page_table_entry(pte_t *entry, uint8_t access,
                          uint32_t address) {
    entry->access = access;
    entry->ignored = 0x0u;
    entry->page_addr = address;
}

void u_page_dir_init(pde_t *page_dir) {
    uint32_t i = 0;
    // set user space
    for (; i < U_PDE_NUM; ++i) {
        page_dir[i].ignored = 0x0u;
        page_dir[i].access = U_PDE_WRITE_ABSENT_ACCESS;
        page_dir[i].page_table_addr = PTE_ABSENT_ADDR;
    }
    // copy kernel space's page dir entry
    for (; i < PDE_NUM; ++i) {
        pde_t *k_page_dir = (pde_t *) K_PD_ADDR;
        page_dir[i].ignored = k_page_dir[i].ignored;
        page_dir[i].access = k_page_dir[i].access;
        page_dir[i].page_table_addr = k_page_dir[i].page_table_addr;
    }
}

void u_release_pages(pte_t *page_table) {
    for (uint32_t i = 0; i < PTE_MAX_NUM; ++i) {
        if (page_table[i].page_addr != PTE_ABSENT_ADDR) {
            release_page(page_no(page_table[i].page_addr));
        }
    }
}

void pf_handler(intr_reg_t *intr_reg) {
    // '%CR2' contains linear address that generated the exception.
    uint32_t addr = get_cr(2);
    // '%CR3' contains the physical address of page directory.
    uint32_t cr3 = direct_mapping_virtual_addr(get_cr(3));
    uint32_t err_code = intr_reg->err_code;

    // reference an absent page
    if (is_bit_set(err_code, 0) == FALSE) {
        pde_t *pd = (pde_t *) cr3;
        uint16_t pd_i = addr / PT_MAX_SIZE;
        if (pd[pd_i].page_table_addr == PDE_ABSENT_ADDR) {
            // page directory entry in absent state
            pd[pd_i].page_table_addr = direct_mapping_phys_addr(request_bytes(sizeof(pte_t)));
            pd[pd_i].ignored = 0x0u;
            if (is_bit_set(err_code, 1) == TRUE) {
                pd[pd_i].access = U_PDE_WRITE_PRESENT_ACCESS;
            } else {
                pd[pd_i].access = U_PDE_READ_PRESENT_ACCESS;
            }
        }
        // set page table for absent page
        pte_t *pt = (pte_t *) pd[pd_i].page_table_addr;
        uint16_t pt_i = (addr << 10u >> 10u) / PAGE_SIZE;
        pt[pt_i].ignored = 0x0u;
        pt[pt_i].page_addr = page_phys_addr(pd_i * PTE_MAX_NUM + pt_i);
        if (is_bit_set(err_code, 1) == TRUE) {
            pd[pd_i].access = U_PTE_WRITE_PRESENT_ACCESS;
        } else {
            pd[pd_i].access = U_PTE_READ_PRESENT_ACCESS;
        }
        return;
    }
    // user-mode access
    if (is_bit_set(err_code, 2) == TRUE) {
        // It's a severe invalid access and we cannot fix it, so we close the process enforcedly.
        close_proc((pcb_t *) (U_STACK_ADDR + sizeof(pcb_t)));
        return;
    }
    // TODO: If more possible sources are added, we cannot determine whether it's a W/R fault like this.
    // try to write to a read-only page
    if (is_bit_set(err_code, 1) == TRUE) {
        close_proc((pcb_t *) (U_STACK_ADDR + sizeof(pcb_t)));
        return;
    }
}