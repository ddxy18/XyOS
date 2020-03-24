//
// Created by dxy on 2020/3/22.
//

/**
 * Direct mapping allocator manages kernel's direct mapping areas. It will firstly request 512 MB from page allocator
 * and set corresponding page tables.
 * // allocation size: 1 byte, 2 bytes, 4 bytes, 8 bytes, 16 bytes, 32 bytes, 64 bytes, 128 bytes, 256 bytes, 512 bytes
 * 1 KB, 2 KB, 4 KB, 8 KB, 16 KB, 32 KB, 64 KB, 128 KB, 256 KB, 512 KB
 * 1 MB, 2 MB, 4 MB, 8 MB, 16 MB, 32 MB
 */
#ifndef XYOS_DIRECT_MAPPING_ALLOCATOR_H
#define XYOS_DIRECT_MAPPING_ALLOCATOR_H

#include <stdint.h>

#include "../lib/def.h"

#define MEM_SLICE_TYPE_NUM 26u
// 512 MB direct mapping area
#define DIRECT_MAPPING_SIZE 0x20000000u
#define NODE_POOL_SIZE 0x400000u

// translate between kernel direct mapping area's physical address and virtual address
#define direct_mapping_virtual_addr(physical_addr) physical_addr + K_VIRTUAL_ADDR
#define direct_mapping_physical_addr(virtual_addr) virtual_addr - K_VIRTUAL_ADDR

/**
 * translate array number to size in bytes
 */
#define type_to_size(type) (1u << (type))

/**
 * Request 512 MB from page allocator, open paging and initialize the direct mapping allocator's fundamental structures.
 */
void direct_mapping_init();

/**
 * @param size request 'size' bytes continuous area
 * @return virtual address of the allocated area, if a failure happens, return 0
 */
pointer_t request_bytes(uint32_t size);

/**
 * Request 'size' bytes starting from 'virtual_addr'.
 * If any byte in the requested slice has been allocated, allocation will fail.
 * If 'size' is not multiple of 2, it will allocate a slice larger than 'size' to satisfy the size requirement.
 * So if the extra aligned area have been allocated before, allocation will also fail.
 * This function is mainly used to store some global data structure,
 * such as GDT and PD in fixed areas, when initializing the kernel.
 * @param virtual_addr the smallest physical address of the requested area
 * @param size request 'size' bytes continuous area
 * @return virtual address of the allocated area, if a failure happens, return -1
 */
int64_t request_fixed_bytes(pointer_t virtual_addr, uint32_t size);

/**
 * @param virtual_addr the smallest virtual address of the released area
 */
void release_bytes(pointer_t virtual_addr);

#endif //XYOS_DIRECT_MAPPING_ALLOCATOR_H