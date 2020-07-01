//
// Created by dxy on 2020/3/17.
//

/**
 * Now it only supports 4 KB page sizes and page cache is disabled. Kernel's
 * page directory locates in 0x1000~0x1fff.
 */

#include "paging.h"

#include <stddef.h>

#include "../driver/console.h"
#include "../lib/bitmap.h"
#include "memory_manager.h"
#include "../lib/asm_wrapper.h"
#include "../lib/bit.h"
#include "../process/process.h"

// now only support 4 KB pages
static const int PageSize = 0x1000;

// 4 GB memory space contains 2^20 pages.
static const int kVirtualPageNum = 0x100000;
// first physical page after kernel's 64 MB direct mapping area
static const int kDynamicMappingStartPage = 0x4000;
// a maximum of 64 MB continuous physical memory in kernel dynamic mapping area
static const int kKMaxContinuousPageNum = 0x4000;

// page directory's global properties
static const int kUPdeNum = 768;
static const int kKPdeNum = 256;
static const int kKPdeDirectMappingNum = 16;

// page table's global properties
static const int kPteMaxNum = 1024;
static const int kPteSize = 4;
static const int kPtMaxSize = kPteMaxNum * kPteSize;
static const uintptr_t PTE_ABSENT_ADDR = 0;
static const uintptr_t kKPtAddr = 0xc000d000;

// PDE and PTE Access options, cache policy is disabled
// PDE
static const int kKPdeReadPresentAccess = 0x1;
static const int kKPdeWritePresentAccess = 0x3;
static const int kKPdeReadAbsentAccess = 0x0;
static const int kKPdeWriteAbsentAccess = 0x2;
static const int kUPdeReadPresentAccess = 0x5;
static const int kUPdeWritePresentAccess = 0x7;
static const int kUPdeReadAbsentAccess = 0x4;
static const int kUPdeWriteAbsentAccess = 0x6;
// PTE
static const int kKPteReadPresentAccess = 0x1;
static const int kKPteWritePresentAccess = 0x3;
static const int kKPteReadAbsentAccess = 0x0;
static const int kKPteWriteAbsentAccess = 0x2;
static const int kUPteReadPresentAccess = 0x5;
static const int kUPteWritePresentAccess = 0x7;
static const int kUPteReadAbsentAccess = 0x4;
static const int kUPteWriteAbsentAccess = 0x6;

/**
 * Use a bitmap to record whether physical page n is allocated. 0 means free and
 * 1 means allocated.
 */
unsigned int page_state[0x8000];

static void SetPte(Pte *entry, uint8_t access, uintptr_t address);

static void SetPde(Pde *entry, uint8_t access, uintptr_t address);

/**
 * @brief Set page 'n' to free state and clear all bytes to 0.
 *
 * If page 'n' is not available or in free state, this function does nothing.
 *
 * @param n nth physical page
 */
static void RelPage(int n);

/**
 * @return uintptr_t Physical address of the allocated page. If no pages
 * available, it returns 0.
 */
static uintptr_t ReqPage();

/**
 * @return bool Return 'FALSE' if no available memory exists.
 */
static void PageAllocatorInit();

/**
 * translate memory sizes represented in bytes to 4 KB page amounts
 * 'byte_size' must be a multiple of 'PageSize'
 */
static inline int GetSizeInPage(uint32_t byte_size) {
    return (int) (byte_size / (unsigned) PageSize);
}

// calculate page n's address
static inline int GetPagePhysAddr(int n) {
    return n * PageSize;
}

// calculate which page 'addr' locates in
static inline int GetPageNo(uintptr_t addr) {
    return (int) (addr / PageSize);
}

void PagingInit() {
    PageAllocatorInit();
    // set '%cr3'
    SwitchPd(kKPdAddr);

    Pde *k_pd = (Pde *) DirectMappingPhysAddr(kKPdAddr);

    /**
     * Map first 4 MB memory to the same physical address. So when opening
     * paging, current address can also be used. This page table is a
     * temporary page table and will be removed when completing paging
     * initialization.
     */
    uintptr_t pt_addr = DirectMappingPhysAddr(kKPtAddr - kPtMaxSize);
    SetPde(&k_pd[0], kKPdeWritePresentAccess, pt_addr);
    Pte *pt = (Pte *) pt_addr;
    for (int j = 0; j < kPteMaxNum; ++j) {
        SetPte(&pt[j], kKPteWritePresentAccess, j * PageSize);
    }

    // set user space's pde to absent state
    int i = 1;
    for (; i < kUPdeNum; ++i) {
        SetPde(&k_pd[i], kUPdeWriteAbsentAccess, kPdeAbsentAddr);
    }

    // initialize direct mapping area
    pt_addr = DirectMappingPhysAddr(kKPtAddr);
    uintptr_t page_addr = 0x0u;
    for (; i < kKPdeDirectMappingNum + kUPdeNum; ++i) {
        SetPde(&k_pd[i], kKPdeWritePresentAccess, pt_addr);

        pt = (Pte *) pt_addr;
        for (int j = 0; j < kPteMaxNum; ++j) {
            SetPte(&pt[j], kKPteWritePresentAccess, page_addr);
            page_addr += PageSize;
        }
        pt_addr += kPtMaxSize;
    }

    // set all dynamic mapping area's pde to absent state
    for (; i < kPdeNum; ++i) {
        SetPde(&k_pd[i], kKPdeWriteAbsentAccess, kPdeAbsentAddr);
    }

    // set CR0.PG = 1 and CR4.PAE = 0 to open 32-bit paging
    clear_cr(4, 5);
    set_cr(0, 31);

    // TODO: Update all registers containing addresses to virtual addresses and remove first 4 MB mapping.
    // set '%esp' to its corresponding virtual address
    // asm("addl %0, %%esp"::"r"(kKVirtualAddr));

    // refresh '%eip'
    /*uint16_t cs;
    asm("movw %%cs, %0":"=r"(cs)::"memory");
    ljmp(cs, kKVirtualAddr);
    asm("addl %0, %%eip"::"r"(kKVirtualAddr));*/
    // remove first 4 MB mapping
    /*k_pd = (Pde *) kKPdAddr;
    k_pd[0].access = kPdeRead_AbsentAccess;
    pt = (Pte *) DirectMappingVirtualAddr(k_pd[0].pt_addr);
    k_pd[0].pt_addr = kPdeAbsentAddr;
    for (uint16_t j = 0; j < kPteMaxNum; ++j) {
        pt[j].Access = kPteRead_AbsentAccess;
        pt[j].GetPageAddrInPte = PTE_ABSENT_ADDR;
    }*/
}

void SwitchPd(uintptr_t pd_addr) {
    uintptr_t phys_addr = DirectMappingPhysAddr(pd_addr);
    /**
     * Now 'Access' and 'ignored' are all 0. And 'pd_addr' lower 12 bits are
     * all 0. So we can directly put 'pd_addr' to '%cr3'.
     */
    asm("movl %0, %%eax"::"r"(phys_addr));
    asm("movl %eax, %cr3");
}

/**
 * It changes all pages in 'father_pd' to read-only and copy a pair for
 * 'cur_pd'. That means we use copy on write for child process.
 *
 * @param cur_pd
 * @param father_pd
 */
void UPdClone(Pde *cur_pd, Pde *father_pd) {
    for (int i = 0; i < kUPdeNum; ++i) {
        // clear R/W flag in page directory
        father_pd[i].access &= ~0x2u;
    }
    for (int i = 0; i < kPdeNum; ++i) {
        cur_pd[i].ignored = father_pd[i].ignored;
        cur_pd[i].access = father_pd[i].access;
        cur_pd[i].pt_addr = father_pd[i].pt_addr;
    }
}

void RelPages(Pte *pt) {
    for (int i = 0; i < kPteMaxNum; ++i) {
        if (pt[i].page_addr != PTE_ABSENT_ADDR) {
            RelPage(GetPageNo(pt[i].page_addr << 12u));
        }
    }
}

void PfHandler(IntrReg *intr_reg) {
    // '%CR2' contains linear address that generated the exception.
    uintptr_t addr = get_cr(2);
    // '%Cr3' contains the physical address of page directory.
    uint32_t cr3 = DirectMappingVirtualAddr(get_cr(3));
    uint32_t err_code = intr_reg->err_code;

    // reference an absent page
    if (IsBitSet(err_code, 0) == FALSE) {
        Pde *pd = (Pde *) cr3;
        int pd_i = (int) addr / (kPteMaxNum * PageSize);
        if (pd[pd_i].pt_addr == kPdeAbsentAddr) {
            // page directory entry in absent state
            if (IsBitSet(err_code, 1) == TRUE) {
                SetPde(&pd[pd_i], kUPdeWritePresentAccess, DirectMappingPhysAddr
                        (ReqBytes(kPtMaxSize)));
            } else {
                SetPde(&pd[pd_i], kUPdeReadPresentAccess, DirectMappingPhysAddr
                        (ReqBytes(kPtMaxSize)));
            }
        }
        // set page table for absent page
        Pte *pt = (Pte *) (pd[pd_i].pt_addr << 12u);
        int pt_i = GetPageNo(addr) % kPteMaxNum;
        if (IsBitSet(err_code, 1) == TRUE) {
            SetPte(&pt[pt_i], kUPteWritePresentAccess, ReqPage());
        } else {
            SetPte(&pt[pt_i], kUPteReadPresentAccess, ReqPage());
        }
        return;
    }
    // user-mode Access
    if (IsBitSet(err_code, 2) == TRUE) {
        // We cannot fix it, so we release the process enforcedly.
        CloseProc();
        return;
    }
    // TODO: If more possible sources are added, we cannot determine whether
    //  it's a W/R fault like this.
    // try to write to a read-only page
    if (IsBitSet(err_code, 1) == TRUE) {
        CloseProc();
        return;
    }
}

/**
 * Cr3 only reserve 20 bits to store page directory's physical address, so
 * page directory will be 4 KB aligned and Cr3 only store the upper 20 bits.
 */
static inline uintptr_t GetPdAddrInReg(uintptr_t phys_addr) {
    return phys_addr >> 12u;
}

/**
 * PDE only reserve 20 bits to store physical table's physical address, so
 * page table will be 4 KB aligned and PDE only store the upper 20 bits.
 */
static inline uintptr_t GetPtAddrInPde(uintptr_t phys_addr) {
    return phys_addr >> 12u;
}

/**
 * PDE only store the upper 20 bits of the physical page's physical address.
 */
static inline uintptr_t GetPageAddrInPte(uintptr_t phys_addr) {
    return phys_addr >> 12u;
}

static void SetPte(Pte *entry, uint8_t access, uintptr_t address) {
    entry->access = access;
    entry->ignored = 0;
    entry->page_addr = GetPageAddrInPte(address);
}

static void SetPde(Pde *entry, uint8_t access, uintptr_t address) {
    entry->access = access;
    entry->ignored = 0;
    entry->pt_addr = GetPtAddrInPde(address);
}

/**
 * Page allocator is a physical page allocator which manages all available
 * physical pages' allocation and release. It's a public basic memory
 * allocator and all other memory managers exclude direct mapping allocator
 * works on the basic of it.
 * 'avl_mem' is initialized in 'start.c', so we get all available memory's
 * physical address and size. We use 'page_state' to record whether physical
 * page n is allocated.
 * 'ReqPage' allocates a page and its physical address is random. User space
 * and kernel dynamic mapping area requests memory from the allocator through
 * page faults.
 * Notice that all memory addresses in this allocator are physical addresses.
 * Translation to virtual address and modification of page tables should be
 * completed by requester themselves.
 */

/**
 * @brief Set all bytes in this page to 0.
 *
 * @param n nth physical page
 */
static void ClearPage(int n);

/**
 * It sets 'page_state' according to 'available_mem'. If any bytes in a page
 * is unavailable, this page is viewed as an unavailable page. All
 * unavailable pages' bits are set to 1 and others are set to 0.
 */
static void PageAllocatorInit() {
    AvlPhysMem *t = GetAvlMem();

    if (t->size == UINT32_MAX) {
        printf("No available memory for kernel, some error may happen in "
               "bootloader.\nXyOS will stop now.\n");
        // TODO: reset OS
    }

    uintptr_t addr = 0;
    while (t->size != UINT32_MAX) {
        /**
         * Set all pages between two available memory blocks as unavailable
         * pages.
         */
        while (addr < t->addr) {
            SetBitmap(page_state, GetPageNo(addr));
            addr += PageSize;
        }
        /**
         * set all pages completely in an available memory slice as available
         * pages.
         */
        while (addr + PageSize <= t->addr + t->size) {
            ClearBitmap(page_state, GetPageNo(addr));
            addr += PageSize;
        }
        t++;
    }
    t = NULL;
    /**
     * Set all pages haven't checked until now as unavailable pages.
     * After checking the last page, 'addr' will overflow to 0, so we use 0
     * as a sign of the end.
     */
    while (GetPageNo(addr) != 0) {
        ClearBitmap(page_state, GetPageNo(addr));
        addr += PageSize;
    }

    /**
     * Set first 64 MB as available, so direct mapping allocator can manage
     * them. We have checked minimum memory size in 'init.c', so we don't
     * need to worry about that available memory is less than 64 MB.
     */
    for (uint32_t i = 0; i < kDynamicMappingStartPage; i++) {
        ClearBitmap(page_state, i);
    }
}

/**
 * It searches 'page_state' from 'kDynamicMappingStartPage' in sequence until
 * it finds the first free page.
 */
static uintptr_t ReqPage() {
    for (uint32_t i = kDynamicMappingStartPage; i < kVirtualPageNum; ++i) {
        if (!IsBitmapSet(page_state, i)) {
            SetBitmap(page_state, i);
            return GetPagePhysAddr(i);
        }
    }
    return 0;
}

static void RelPage(int n) {
    uintptr_t addr = GetPagePhysAddr(n);
    AvlPhysMem *t = GetAvlMem();
    while (t->size != -1) {
        if (t->addr < addr && t->addr + t->size >= addr + PageSize) {
            ClearPage(n);
            ClearBitmap(page_state, n);
            t = NULL;
            return;
        }
        t++;
    }
    t = NULL;
}

static void ClearPage(int n) {
    uintptr_t physical_addr = GetPagePhysAddr(n);
    uintptr_t *byte_pointer = (uintptr_t *) physical_addr;
    for (; (uintptr_t) byte_pointer <
           physical_addr + PageSize; ++byte_pointer) {
        *byte_pointer = 0;
    }
}