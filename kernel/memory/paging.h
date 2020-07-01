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


// page directory's properties
static const uintptr_t kKPdAddr = 0xc0001000;

static const int kPdeNum = 1024;
static const int kPdeSize = 4;
static const int kPdSize = kPdeNum * kPdeSize;
static const uintptr_t kPdeAbsentAddr = 0;

/**
 * @typedef Pde page directory entry
 */
typedef struct Pde {
    uint8_t access;
    uint8_t ignored: 4;
    uintptr_t pt_addr: 20;
} __attribute__((packed)) Pde;

/**
 * @typedef Pte page table entry
 */
typedef struct Pte {
    uint16_t access: 9;
    uint8_t ignored: 3;
    uintptr_t page_addr: 20;
} __attribute__((packed)) Pte;

typedef struct Cr3 {
    uint8_t access: 5;
    uint8_t ignored: 7;
    uintptr_t pd_addr: 20;
} Cr3;

/**
 * Initialize kernel's page directory and page tables. Map 64 MB direct mapping
 * area to the memory space starting from 0x0. Enable paging.
 */
void PagingInit();

/**
 * @brief Switch page directory to the one located in 'pd_addr'.
 *
 * @verbatim
 * Two possible cases may need this function:
 *      - process switch
 *      - process release
 * @endverbatim
 *
 * @param pd_addr virtual address of the new page directory
 */
void SwitchPd(uintptr_t pd_addr);

/**
 * Creating a new process uses this function to set its page directory and
 * change father's page directory.
 *
 * @param cur_pd
 * @param father_pd
 */
void UPdClone(Pde *cur_pd, Pde *father_pd);

/**
 * @brief Release all pages exist in 'pt'.
 *
 * It modifies corresponding page directory entry for you. But you have to
 * release 'pt' yourself.
 *
 * @param pt
 */
void RelPages(Pte *pt);

/**
 * @brief page fault handler
 *
 * @verbatim
 * Now there are only 3 possible sources of PF exception.
 * 1. page not present -- allocate a new page
 * 2. invalid mode: -- release the process enforcedly
 *      - kernel Access address below than 0xc0000000u
 *      - user Access address above than 0xc0000000u
 * 3. write to a read-only page -- release the process enforcedly
 * @endverbatim
 *
 * @param intr_reg
 */
void PfHandler(IntrReg *intr_reg);

#endif // XYOS_PAGING_H