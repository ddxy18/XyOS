//
// Created by dxy on 2020/3/22.
//

/**
 * Direct mapping allocator manages kernel direct mapping area. When
 * initializing, it requests 64 MB from page allocator and reserves fixed
 * memory for some global data structure and kernel image. Now, it can start
 * managing remaining free memory and dynamically allocate and release bytes
 * as needed. It uses buddy algorithm to manage free memory blocks.
 * Allocation sizes are as follows: 1 byte, 2 bytes, 4 bytes, 8 bytes, 16
 * bytes, 32 bytes, 64 bytes, 128 bytes, 256 bytes, 512 bytes 1 KB, 2 KB, 4
 * KB, 8 KB, 16 KB, 32 KB, 64 KB, 128 KB, 256 KB, 512 KB, 1 MB, 2 MB, 4 MB, 8
 * MB, 16 MB, 32 MB.
 */

#include "direct_mapping_allocator.h"

#include <stddef.h>

#include "../lib/list.h"
#include "segment.h"

#define kMemBlockTypeNum  26
// 1 MB memory used to manage kernel direct mapping memory's allocation
static const int kNodePoolSize = 0x100000;
// 64 MB direct mapping area
static const int kDirectMappingSize = 0x4000000;

typedef struct MemBlock {
    uintptr_t addr;
    ListNode node;
} MemBlock;

/**
 * Since our allocator manages kernel memory allocation, it will be used
 * frequently. So we store enough 'MemBlock' structure in 'mem_block_pool'
 * for the allocator.
 */
static ListNode *mem_block_pool;

// 'MemBlock' in the two lists sort from lower address to higher address.
static ListNode *free_mem_list[kMemBlockTypeNum];
static ListNode *allocated_mem_list[kMemBlockTypeNum];

/**
 * '&K_BEGIN' and '&K_END' are kernel image's begin and end addresses. Notice
 * that the two addresses are all physical addresses.
 */
extern uintptr_t K_BEGIN, K_END;

static void MemBlockPoolInit(uintptr_t addr, int size);

static MemBlock *ReqMemBlock();

static void RelMemBlock(MemBlock *mem_block);

static void MergeBlock(int size_type);

static bool DivBlock(int size_type);

static int GetMemBlockSizeType(int size);

static inline int GetMemBlockSize(int size_type) {
    return (int) (1u << ((unsigned int) size_type));
}

/**
 * Request continuous 64 MB starting from page 0 from page allocator. Reserve
 * memory for kernel image and some global data structure.
 */
void DirectMappingInit() {
    // use last 1MB as a 'MemBlock' pool
    MemBlockPoolInit(
            DirectMappingVirtualAddr(
                    kDirectMappingSize - kNodePoolSize),
            kNodePoolSize);

    // We initialize every list in both arrays, say, we complete work of 'List'.
    for (int i = 0; i < kMemBlockTypeNum; ++i) {
        free_mem_list[i] = &ReqMemBlock()->node;
        free_mem_list[i]->next = free_mem_list[i];
        allocated_mem_list[i] = &ReqMemBlock()->node;
        allocated_mem_list[i]->next = allocated_mem_list[i];
    }

    /**
     * Reserve memory below '&K_END'. This area contains kernel's image,
     * central stack and some data structure to help hardware work properly,
     * so it will not be allocated and released dynamically.
     */
    int size_type = GetMemBlockSizeType((int) &K_END);
    int size = GetMemBlockSize(size_type);
    MemBlock *reserved = ReqMemBlock();
    reserved->addr = kKVirtualAddr;
    ListAdd(allocated_mem_list[size_type], &reserved->node);

    /**
     * Insert all free memory slices to 'free_mem_list'.
     * We divide memory into largest blocks until the remaining memory size
     * is smaller than the block size. Then we try to divide them to smaller
     * blocks. We repeat the work until there is no remaining memory or the
     * remaining memory is smaller than the smallest block size.
     */
    int i = size;
    int j = kMemBlockTypeNum - 1;
    while (j >= 0 && i < kDirectMappingSize - kNodePoolSize) {
        while (i + GetMemBlockSize(j) <= kDirectMappingSize - kNodePoolSize) {
            MemBlock *new_block = ReqMemBlock();
            new_block->addr = DirectMappingVirtualAddr(i);
            ListAdd(free_mem_list[j], &new_block->node);
            i += GetMemBlockSize(j);
        }
        --j;
    }
}

uintptr_t ReqBytes(int size) {
    int size_type = GetMemBlockSizeType(size);

    if (IsListEmpty(free_mem_list[size_type]) == TRUE) {
        // No fit block now. We need to divide a large block to smaller pieces.
        if (DivBlock(size_type + 1) == TRUE) {
            // It divides larger blocks successfully, so we can request again.
            return ReqBytes(size);
        }
        return 0;
    }
    MemBlock *free_block = GET_STRUCT(ListRemove(free_mem_list[size_type]),
                                      MemBlock, node);

    // Insert allocated block into appropriate position in 'allocated_mem_list'.
    ListNode *tmp = allocated_mem_list[size_type];
    while (tmp->next != allocated_mem_list[size_type] &&
           GET_STRUCT(tmp->next, MemBlock, node)->addr < free_block->addr) {
        tmp = tmp->next;
    }
    ListAdd(tmp, &free_block->node);
    return free_block->addr;
}

/**
 * It searches through 'allocated_mem_list' to find size of memory block
 * starting from 'addr' and then removes it from 'allocated_mem_list' and
 * adds it to 'free_mem_list'. Also, it does continuous free blocks check
 * after every release.
 *
 * @param addr
 */
void RelBytes(uintptr_t addr) {
    /**
     * Get allocated area's size according to 'addr' and remove the node from
     * 'allocated_mem_list'.
     */
    ListNode *remove;
    int size_type = 0;
    for (; size_type < kMemBlockTypeNum; ++size_type) {
        remove = allocated_mem_list[size_type];
        if (IsListEmpty(remove) == FALSE) {
            while (remove->next != allocated_mem_list[size_type]) {
                if (GET_STRUCT(remove->next, MemBlock, node)->addr == addr) {
                    remove = ListRemove(remove);
                    goto out;
                }
                if (GET_STRUCT(remove->next, MemBlock, node)->addr > addr) {
                    goto out;
                }
                remove = remove->next;
            }
        }
    }

    out:
    if (size_type == kMemBlockTypeNum) {
        return;
    }

    // insert released block into 'free_mem_list'
    ListNode *tmp = free_mem_list[size_type];
    while (tmp->next != free_mem_list[size_type] && GET_STRUCT(tmp->next,
                                                               MemBlock,
                                                               node)->addr <
                                                    addr) {
        tmp = tmp->next;
    }
    ListAdd(tmp, remove);
    // merge slices into a larger slice
    MergeBlock(size_type);
}

/**
 * Allocate all memory between 'addr' and 'addr + size -1' to
 * 'mem_block_pool'. Use first 'MemBlock' as the head of the List and it will
 * never be allocated.
 *
 * @param addr It should 4 KB aligned.
 * @param size It should be multiple of 'sizeof(MemBlock)'.
 */
static void MemBlockPoolInit(uintptr_t addr, int size) {
    /**
     * We don't use 'List' to initialize the list since it relies on the
     * direct mapping allocator. We complete its work manually.
     */
    mem_block_pool = &((MemBlock *) addr)->node;
    mem_block_pool->next = mem_block_pool;

    MemBlock *tmp = GET_STRUCT(mem_block_pool, MemBlock, node);
    for (; (uintptr_t) tmp < (uintptr_t) GET_STRUCT(mem_block_pool, MemBlock,
                                                    node) + size; tmp++) {
        ListAdd(mem_block_pool, &tmp->node);
    }
}

/**
 * @return If there is no 'MemBlock' in pool, it returns 'NULL'.
 */
static MemBlock *ReqMemBlock() {
    // pool is used up
    if (IsListEmpty(mem_block_pool) == TRUE) {
        return NULL;
    }

    return GET_STRUCT(ListRemove(mem_block_pool), MemBlock, node);
}

static void RelMemBlock(MemBlock *mem_block) {
    mem_block->node.next = NULL;
    ListAdd(mem_block_pool, &mem_block->node);
}

/**
 * Merge continuous blocks of 'size_type' size type into a larger block. If
 * this leads to continuous blocks in 'size_type + 1', it merges them. It
 * continues this work until no continuous blocks remain.
 *
 * @param size_type
 */
static void MergeBlock(int size_type) {
    if (size_type == kMemBlockTypeNum - 1) {
        return;
    }

    int size = GetMemBlockSize(size_type);
    ListNode *tmp = free_mem_list[size_type];
    while (tmp->next != free_mem_list[size_type]) {
        MemBlock *block1 = GET_STRUCT(tmp->next, MemBlock, node);
        MemBlock *block2 = GET_STRUCT(tmp->next->next, MemBlock, node);
        if (tmp->next->next != free_mem_list[size_type] &&
            block1->addr + size == block2->addr) {
            // 'block1' and 'block2' are continuous in address.
            int reserved_size = GetMemBlockSize(GetMemBlockSizeType(
                    (int) &K_END));
            if (((block1->addr - reserved_size) / size) % 2 == 0) {
                // 'block1' and 'block2' are divided from the same block.
                ListRemove(tmp);
                ListRemove(tmp->next);
                RelMemBlock(block2);
                tmp = free_mem_list[size_type + 1];
                while (tmp->next != free_mem_list[size_type + 1] &&
                       GET_STRUCT(tmp->next, MemBlock, node)->addr <
                       block1->addr) {
                    tmp = tmp->next;
                }
                ListAdd(tmp, &block1->node);
                MergeBlock(size_type + 1);
                return;
            }
        }
        tmp = tmp->next;
    }
}

/**
 * If there are blocks in free_mem_list[size_type], divide a blocks to 2
 * blocks. Otherwise search larger blocks.
 * 
 * @param size_type Size type of blocks to be divided. It must be a number
 * from 1 through 'kMemBlockTypeNum - 1'.
 * @return It will return 'TRUE' after successful division. If all larger
 * blocks are used up, it will return 'FALSE'.
 */
static bool DivBlock(int size_type) {
    // invalid size type
    if (size_type == 0 || size_type == kMemBlockTypeNum) {
        return FALSE;
    }

    if (IsListEmpty(free_mem_list[size_type]) == FALSE) {
        // Blocks whose size types are 'size_type' exist.
        MemBlock *new_block1 = GET_STRUCT(
                ListRemove(free_mem_list[size_type]), MemBlock, node);
        MemBlock *new_block2 = ReqMemBlock();
        new_block2->addr = new_block1->addr + GetMemBlockSize(size_type - 1);
        ListAdd(free_mem_list[size_type - 1], &new_block1->node);
        ListAdd(free_mem_list[size_type - 1], &new_block2->node);
        return TRUE;
    } else {
        // search for larger blocks
        if (DivBlock(size_type + 1) == TRUE) {
            return DivBlock(size_type);
        }
        return FALSE;
    }
}

/**
 * Get size_type according to 'size'. If 'size' is not power of 2, find the next
 * number which is power of 2 as 'size'.
 *
 * @param size represented in bytes
 * @return int
 */
static int GetMemBlockSizeType(int size) {
    int i = 0;
    for (; i < 32; ++i) {
        if (((unsigned) size >> (unsigned) i) == 1u) {
            break;
        }
    }
    if (((unsigned) size & (1u << (unsigned) i)) == size) {
        return i;
    }
    return i + 1;
}