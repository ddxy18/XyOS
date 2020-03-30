//
// Created by dxy on 2020/3/22.
//

#include "direct_mapping_allocator.h"
#include "../driver/console.h"
#include "../lib/list.h"
#include "../test/test.h"
#include "page_allocator.h"
#include "paging.h"
#include "segment.h"
#include <stddef.h>

void node_pool_init(pointer_t, uint32_t);

linked_list_node_t *request_node();

void release_node(linked_list_node_t *);

void merge_slice(uint16_t);

uint8_t divide_slice(uint16_t);

uint16_t get_size_type(uint32_t);

// head of a list of unused node which will be heavily used to record slices'
// state
linked_list_node_t *node_pool;
// 'free_mem_list[i]' has a list of 2^i bytes free memory slices.
linked_list_t free_mem_list[MEM_SLICE_TYPE_NUM];
// in a sequence from lower address to upper address
linked_list_t allocated_mem_list[MEM_SLICE_TYPE_NUM];

void direct_mapping_init() {
    // request first 64 MB from page allocator to create kernel direct mapping
    // area
    if (k_request_pages(0, byte_to_page(DIRECT_MAPPING_SIZE)) == -1) {
        printf("request at least 64 MB for kernel direct mapping area.\n");
        printf("XyOS will stop now.");
        return;
    }

    // use last 1MB as a 'linked_list_node_t' pool
    node_pool_init(
            direct_mapping_virtual_addr(DIRECT_MAPPING_SIZE - NODE_POOL_SIZE),
            NODE_POOL_SIZE);

    // set head as a null node which helps to simplify list operation
    for (uint16_t i = 0; i < MEM_SLICE_TYPE_NUM; ++i) {
        free_mem_list[i].head = request_node();
        allocated_mem_list[i].head = request_node();
    }

    // Reserve memory below 'K_END'. This area contains kernel's image, cental
    // stack and some data structure to help hardware work properly, so it will
    // not be allocated and released dynamically.
    uint16_t size_type = get_size_type((uint32_t) &K_END);
    uint32_t size = type_to_size(size_type);
    linked_list_node_t *reserved_node = request_node();
    allocated_mem_list[size_type].head->next = reserved_node;
    reserved_node->data = K_VIRTUAL_ADDR;

    // insert all free memory slices to the last list in 'free_mem_list'
    uint32_t i = size;
    uint16_t j = MEM_SLICE_TYPE_NUM - 1;
    linked_list_node_t *tmp = free_mem_list[j].head;
    while (j >= 0 && i < DIRECT_MAPPING_SIZE - NODE_POOL_SIZE) {
        tmp = free_mem_list[j].head;
        while (i + type_to_size(j) <= DIRECT_MAPPING_SIZE - NODE_POOL_SIZE) {
            tmp->next = request_node();
            tmp = tmp->next;
            tmp->data = direct_mapping_virtual_addr(i);
            i += type_to_size(j);
        }
        --j;
    }
}

pointer_t request_bytes(uint32_t size) {
    uint16_t size_type = get_size_type(size);

    linked_list_node_t *free_slice = free_mem_list[size_type].head;
    if (free_slice->next != NULL) {
        // find a fit slice
        free_slice = free_slice->next;
        free_mem_list[size_type].head->next = free_slice->next;
        free_slice->next = NULL;
    } else {
        // no fit slice now, need to divide a larger slice to small pieces
        if (divide_slice(size_type + 1) == 0) {
            // divide larger slices successfully and request again
            return request_bytes(size);
        }
        return 0;
    }

    // insert allocated slice into appropriate allocated_mem_list
    linked_list_node_t *tmp = allocated_mem_list[size_type].head;
    while (tmp->next != NULL && tmp->next->data < free_slice->data) {
        tmp = tmp->next;
    }
    free_slice->next = tmp->next;
    tmp->next = free_slice;
    return free_slice->data;
}

void release_bytes(pointer_t addr) {
    // get allocated area's size according to 'physical_addr' and remove the node
    // from 'allocated_mem_list'
    linked_list_node_t *remove_node;
    uint16_t size_type = 0;
    for (; size_type < MEM_SLICE_TYPE_NUM; ++size_type) {
        remove_node = allocated_mem_list[size_type].head;
        while (remove_node->next != NULL) {
            if (remove_node->next->data == addr) {
                // remove node
                linked_list_node_t *t = remove_node;
                remove_node = remove_node->next;
                remove_node->next = NULL;
                t->next = t->next->next;
                t = NULL;
                break;
            }
            if (remove_node->next->data > addr) {
                break;
            }
            remove_node = remove_node->next;
        }
    }

    if (size_type == MEM_SLICE_TYPE_NUM) {
        return;
    }

    // insert released slice into 'free_mem_list'
    linked_list_node_t *tmp = free_mem_list[size_type].head;
    while (tmp->next != NULL && tmp->next->data < addr) {
        tmp = tmp->next;
    }
    remove_node->next = tmp->next;
    tmp->next = remove_node;
    // merge slices into a larger slice
    merge_slice(size_type);
}

/**
 * @brief Allocate all memory between 'begin_virtual_addr' and
 * 'begin_virtual_addr + size -1' to 'node_pool'. Use first 8 bytes as the head
 * of the empty node list and it will never be allocated. Every time we need an
 * empty node, we just search the second node.It can greatly decrease time to
 * find empty node. Now paging is disabled and segment provided by bootloader
 * will translate logical address to the same physical address, so we can use
 * physical address directly.
 * @param begin_virtual_addr should be aligned to 4 KB
 * @param size should be multiple of 4 KB
 */
void node_pool_init(pointer_t begin_addr, uint32_t size) {
    uint8_t node_size = sizeof(linked_list_node_t);
    node_pool = (linked_list_node_t *) begin_addr;

    linked_list_node_t *tmp = node_pool;
    for (pointer_t i = (pointer_t) (node_pool + 1); i < (uint32_t) node_pool + size;
         i += node_size) {
        tmp->next = (linked_list_node_t *) i;
        tmp->next->data = 0;
        tmp = tmp->next;
    }
    // set the last empty node's field 'next' to 'NULL' to ensure
    tmp->next = NULL;
    tmp = NULL;
}

/**
 * @return an unused node. If all node except 'node_pool' is in use, return
 * 'NULL'.
 */
linked_list_node_t *request_node() {
    // node pool is used up
    if (node_pool->next == NULL) {
        return NULL;
    }

    linked_list_node_t *tmp = node_pool->next;
    node_pool->next = node_pool->next->next;
    tmp->next = NULL;
    return tmp;
}

void release_node(linked_list_node_t *node) {
    node->data = 0;
    node->next = node_pool->next;
    node_pool->next = node;
}

/**
 * Merge continuous slices into a larger slice until no continuous slices
 * remain.
 * @param size_type
 */
void merge_slice(uint16_t size_type) {
    if (size_type == MEM_SLICE_TYPE_NUM - 1) {
        return;
    }

    uint32_t size = type_to_size(size_type);
    linked_list_node_t *tmp = free_mem_list[size_type].head;
    while (tmp->next != NULL) {
        if (tmp->next->next != NULL &&
            tmp->next->data + size == tmp->next->next->data) {
            linked_list_node_t *merge_node = tmp->next;
            tmp->next = tmp->next->next->next;
            merge_node->data += size;
            release_node(merge_node->next);
            merge_node->next = NULL;
            tmp = free_mem_list[size_type + 1].head;
            while (tmp->next != NULL && tmp->next->data < merge_node->data) {
                tmp = tmp->next;
            }
            merge_node->next = tmp->next;
            tmp->next = merge_node;
            merge_slice(size_type + 1);
            return;
        }
        tmp = tmp->next;
    }
}

/**
 * If there are slices in free_mem_list[begin_size_type], divide a slice to 2
 * holes. Otherwise search larger slices.
 * @param begin_size_type must be a number from  1 through MEM_SLICE_TYPE_NUM -
 * 1
 * @return It will return 0 after successful division. If all larger slices are
 * used up, it will return 1.
 */
uint8_t divide_slice(uint16_t begin_size_type) {
    // invalid size type
    if (begin_size_type == 0 || begin_size_type == MEM_SLICE_TYPE_NUM) {
        return 1;
    }

    linked_list_node_t *tmp = free_mem_list[begin_size_type].head;
    if (tmp->next != NULL) {
        linked_list_node_t *new_slice = request_node();
        new_slice->data = tmp->next->data + type_to_size(begin_size_type - 1u);
        free_mem_list[begin_size_type - 1].head->next = tmp->next;
        tmp->next = tmp->next->next;
        free_mem_list[begin_size_type - 1].head->next->next = new_slice;
        return 0;
    } else {
        if (divide_slice(begin_size_type + 1) == 0) {
            return divide_slice(begin_size_type);
        }
        return 1;
    }
}

/**
 * Get size_type according to 'size'. If 'size' is not power of 2, find the next
 * number which is power of 2 as 'size'.
 * @param size represent in bytes
 * @return
 */
uint16_t get_size_type(uint32_t size) {
    uint32_t i = 0;
    for (; i < 32; ++i) {
        if ((size >> i) == 1u) {
            break;
        }
    }
    if ((size & (1u << i)) == size) {
        return i;
    }
    return i + 1;
}