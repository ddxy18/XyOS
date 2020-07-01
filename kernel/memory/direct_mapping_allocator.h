//
// Created by dxy on 2020/3/22.
//

#ifndef XYOS_DIRECT_MAPPING_ALLOCATOR_H
#define XYOS_DIRECT_MAPPING_ALLOCATOR_H

#include <stdint.h>

#include "../lib/def.h"
#include "paging.h"

// kernel will use 3GB through 4GB linear address space
static const int kKVirtualAddr = 0xc0000000;

/**
 * @brief translate physical address to virtual address
 *
 * @param physical_addr
 * @return uintptr_t virtual address
 */
static inline uintptr_t DirectMappingVirtualAddr(uintptr_t physical_addr) {
    return physical_addr + kKVirtualAddr;
}

/**
 * @brief translate virtual address to physical address
 *
 * @param virtual_addr
 * @return uintptr_t physical address
 */
static inline uintptr_t DirectMappingPhysAddr(uintptr_t virtual_addr) {
    return virtual_addr - kKVirtualAddr;
}

/**
 * @brief Initialize kernel direct mapping allocator.
 *
 * When we call 'ReqBytes' and 'RelBytes', we must ensure that we have called
 * 'DirectMappingInit' before.
 */
void DirectMappingInit();

/**
 * Allocate 'size' continuous bytes. If 'size' is not a power of 2, it will
 * choose the next number which is power of 2 as 'size'. Caller has no way to
 * get the actual size of the block, so caller should use this block as if it
 * has 'size' bytes.
 *
 * @param size request 'size' continuous bytes
 * @return uintptr_t Return address of the allocated area. If a failure
 * happens, return 0.
 */
uintptr_t ReqBytes(int size);

/**
 * @brief Release the memory block starting from 'addr'.
 *
 * Notice that caller should ensure that the memory block is requested through
 * 'ReqBytes'.
 *
 * @param addr
 */
void RelBytes(uintptr_t addr);

#endif // XYOS_DIRECT_MAPPING_ALLOCATOR_H