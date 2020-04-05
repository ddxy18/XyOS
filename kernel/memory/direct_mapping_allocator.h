//
// Created by dxy on 2020/3/22.
//

/**
 * Direct mapping allocator manages kernel direct mapping area. It will firstly
 * request 64 MB from page allocator. After that, it will set kernel page
 * directory and page tables and reserve fixed memory for some global data
 * structure and kernel image and stack. Now, it can start managing remaining
 * free memory and dynamicly allocate and release bytes as needed. It uses buddy
 * allocator to manage free memory slices. However, it may generate more and
 * more tiny memory slices after a long time since memory combination only
 * combines two continuous memory slices and will not detect whether they are
 * divided from the same larger slice. To create a common interface, all
 * addresses we reference here are virtual addresses.
 * Allocation sizes are as follows: 1 byte, 2 bytes, 4 bytes, 8 bytes, 16 bytes,
 * 32 bytes, 64 bytes, 128 bytes, 256 bytes, 512 bytes 1 KB, 2 KB, 4 KB, 8 KB,
 * 16 KB, 32 KB, 64 KB, 128 KB, 256 KB, 512 KB 1 MB, 2 MB, 4 MB, 8 MB, 16 MB, 32
 * MB.
 */
#ifndef XYOS_DIRECT_MAPPING_ALLOCATOR_H
#define XYOS_DIRECT_MAPPING_ALLOCATOR_H

#include "../lib/def.h"
#include "paging.h"
#include <stdint.h>

#define MEM_SLICE_TYPE_NUM 26u
// 64 MB direct mapping area
#define DIRECT_MAPPING_SIZE 0x4000000u
// 1 MB memory used to manage kernel direct mapping memory's allocation
#define NODE_POOL_SIZE 0x100000u
// 16 KB central stack is responsible for some process unrelated work.
#define CENTRAL_STACK_SIZE 0x4000u
#define CENTRAL_STACK_ADDR 0xc000bc00u

// translate between kernel direct mapping area's physical address and virtual
// address
#define direct_mapping_virtual_addr(physical_addr)                             \
  (physical_addr + K_VIRTUAL_ADDR)
#define direct_mapping_phys_addr(virtual_addr) (virtual_addr - K_VIRTUAL_ADDR)

// translate array number to size in bytes
#define type_to_size(type) (1u << (type))

/**
 * @brief Request continuous 64 MB starting from page 0 from page allocator.
 * Reserve memory for kernel image and central stack and some global data
 * structure. Open paging and modify GDT.
 */
void direct_mapping_init();

/**
 * @brief Allocate 'size' continuous bytes from free memory slices. If size is
 * not a power of 2, it will choose the next number which is power of 2 as
 * 'size'.
 * @param size request 'size' bytes continuous memory
 * @return uintptr_t Return virtual address of the allocated area. If a failure
 * happens, return 0.
 */
uintptr_t request_bytes(uint32_t size);

/**
 * @brief It will search through 'allocated_mem_list' to find size of memory
 * slice starting from 'addr' and then remove it from
 * 'allocated_mem_list' and it to 'free_mem_list'.
 *
 * @param addr the smallest virtual address of the released area
 */
void release_bytes(uintptr_t addr);

// '&K_BEGIN' and '&K_END' are kernel image's begin and end addresses. Notice
// that the two addresses are all physical addresses.
extern uint32_t K_BEGIN, K_END;

#endif // XYOS_DIRECT_MAPPING_ALLOCATOR_H