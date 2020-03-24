//
// Created by dxy on 2page_num2page_num/3/22.
//

#include <stddef.h>
#include "direct_mapping_allocator.h"
#include "page_allocator.h"
#include "paging.h"
#include "../lib/list.h"

void node_pool_init(uint32_t, uint32_t);

linked_list_node_t *request_node();

void release_node(linked_list_node_t *);

void merge_slice(uint16_t);

uint8_t divide_slice(uint16_t);

uint16_t get_size_type(uint32_t);

//
linked_list_node_t *unused_head;
// free_mem_list[i] has a list of same fixed size free memory slices
linked_list_t free_mem_list[MEM_SLICE_TYPE_NUM];
// in a sequence from lower address to upper address
linked_list_t allocated_mem_list[MEM_SLICE_TYPE_NUM];

void direct_mapping_init() {
    // request first 512 MB from page allocator to create kernel direct mapping area
    uint32_t page_num = 0;
    k_request_pages(page_num, K_MAX_CONTINUOUS_PAGE_NUM);
    page_num += K_MAX_CONTINUOUS_PAGE_NUM;
    k_request_pages(page_num, K_MAX_CONTINUOUS_PAGE_NUM);
    page_num += K_MAX_CONTINUOUS_PAGE_NUM;
    k_request_pages(page_num, K_MAX_CONTINUOUS_PAGE_NUM);
    page_num += K_MAX_CONTINUOUS_PAGE_NUM;
    k_request_pages(page_num, K_MAX_CONTINUOUS_PAGE_NUM);

    // use last 4MB as a 'linked_list_node_t' pool
    node_pool_init(direct_mapping_virtual_addr(DIRECT_MAPPING_SIZE - NODE_POOL_SIZE),
                   direct_mapping_virtual_addr(DIRECT_MAPPING_SIZE - 1));

    // set head as a null node which helps to simplify list operation
    for (uint16_t i = 0; i < MEM_SLICE_TYPE_NUM; ++i) {
        free_mem_list[i].head = request_node();
        allocated_mem_list[i].head = request_node();
    }

    // insert all free memory slices to the last list in 'free_mem_list'
    uint32_t i = 0;
    linked_list_node_t *tmp = free_mem_list[MEM_SLICE_TYPE_NUM - 1].head;
    while (i < DIRECT_MAPPING_SIZE - NODE_POOL_SIZE) {
        tmp->next = request_node();
        tmp->next->data = i;
        tmp = tmp->next;
        i += type_to_size(MEM_SLICE_TYPE_NUM - 1u);
    }

    // open paging
    k_paging_init();
}

pointer_t request_bytes(uint32_t size) {
    uint16_t size_type = get_size_type(size);
    linked_list_node_t *free_slice = free_mem_list[size_type].head;
    if (free_slice->next != NULL) {
        // find a fit slice
        free_mem_list[size_type].head->next = free_slice->next->next;
        free_slice = free_slice->next;
        free_slice->next = NULL;
    } else {
        // no fit slice now, need to divide a larger slice to small pieces
        if (divide_slice(size_type + 1) == 0) {
            // divide larger slices successfully and request again
            request_bytes(size);
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
    return direct_mapping_virtual_addr(free_slice->data);
}

int64_t request_fixed_bytes(pointer_t virtual_addr, uint32_t size) {
    pointer_t physical_addr = direct_mapping_physical_addr(virtual_addr);
    uint16_t size_type = get_size_type(size);
    linked_list_node_t *tmp = free_mem_list[size_type].head;

    /**
     * Search 'free_mem_list' in given 'size_type' for free slice begin in 'virtual_addr'.
     * Since every release will merge continuous slices into a large slice, we have not to worry about slice fragments.
     */
    while (tmp->next != NULL && tmp->next->data < physical_addr) {
        tmp = tmp->next;
    }

    if (tmp != NULL) {
        // successfully find the requested slice
        linked_list_node_t *free_slice = tmp->next;
        tmp->next = free_slice->next;

        // insert allocated slice into appropriate allocated_mem_list
        linked_list_node_t *t = allocated_mem_list[size_type].head;
        while (t->next != NULL && t->next->data < free_slice->data) {
            t = t->next;
        }
        free_slice->next = t->next;
        t->next = free_slice;
        return direct_mapping_virtual_addr(free_slice->data);
    } else {
        // some bytes in the requested slice have been allocated
        return -1;
    }
}

void release_bytes(pointer_t virtual_addr) {
    pointer_t physical_addr = direct_mapping_physical_addr(virtual_addr);
    // get allocated area's size according to 'physical_addr' and remove the node from 'allocated_mem_list'
    linked_list_node_t *remove_node;
    uint16_t size_type = 0;
    for (; size_type < MEM_SLICE_TYPE_NUM; ++size_type) {
        remove_node = allocated_mem_list[size_type].head;
        while (remove_node->next != NULL) {
            if (remove_node->next->data == physical_addr) {
                // remove node
                linked_list_node_t *t = remove_node;
                remove_node = remove_node->next;
                remove_node->next = NULL;
                t->next = t->next->next;
                t = NULL;
                break;
            }
            if (remove_node->next->data > physical_addr) {
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
    while (tmp->next != NULL && tmp->next->data < physical_addr) {
        tmp = tmp->next;
    }
    remove_node->next = tmp->next;
    tmp->next = remove_node;
    // merge slices into a larger slice
    merge_slice(size_type);
}

/**
 * Initialize all memory between 'begin_virtual_addr' and 'end_virtual_addr' to 'linked_list_node_t'.
 * Use first 8 bytes as the head of the empty node list and it will never be allocated. It can decrement time to find unused node.
 * "'end_virtual_addr' - 'begin_virtual_addr' + 1" must be a multiple of 8.
 * @param begin_virtual_addr should be aligned to 8 bytes
 * @param end_virtual_addr should be aligned to 8 bytes
 */
void node_pool_init(uint32_t begin_virtual_addr, uint32_t end_virtual_addr) {
    uint8_t node_size = sizeof(linked_list_node_t);
    unused_head = (linked_list_node_t *) begin_virtual_addr;
    linked_list_node_t *tmp;
    for (uint32_t i = begin_virtual_addr; i < end_virtual_addr; i += node_size) {
        tmp = (linked_list_node_t *) i;
        tmp->data = 0;
        tmp->next = tmp + node_size;
    }
    tmp->next = NULL;
    tmp = NULL;
}

/**
 * @return an unused node. If all node except 'unused_head' is in use, return 'NULL'.
 */
linked_list_node_t *request_node() {
    // node pool is used up
    if (unused_head->next == NULL) {
        return NULL;
    }

    linked_list_node_t *tmp = unused_head->next;
    unused_head->next = unused_head->next->next;
    return tmp;
}

void release_node(linked_list_node_t *node) {
    node->data = 0;
    node->next = unused_head->next;
    unused_head->next = node;
}

/**
 * Merge continuous slices into a larger slice until no continuous slices remain.
 * @param size_type
 */
void merge_slice(uint16_t size_type) {
    if (size_type == MEM_SLICE_TYPE_NUM - 1) {
        return;
    }

    uint32_t size = type_to_size(size_type);
    linked_list_node_t *tmp = free_mem_list[size_type].head;
    while (tmp->next != NULL) {
        if (tmp->next->next != NULL && tmp->next->data + size == tmp->next->next->data) {
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
 * If there are slices in free_mem_list[begin_size_type], divide a slice to 2 holes. Otherwise search larger slices.
 * @param begin_size_type must be a number from  1 through MEM_SLICE_TYPE_NUM - 1
 * @return It will return 0 after successful division. If all larger slices are used up, it will return 1.
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
        return divide_slice(begin_size_type + 1);
    }
}

/**
 * Get size_type according to 'size'. If 'size' is not power of 2, find the next number which is power of 2 as 'size'.
 * @param size represent in bytes
 * @return
 */
uint16_t get_size_type(uint32_t size) {
    uint32_t i = 0;
    for (; i < 32; ++i) {
        if ((size << i) == 1u) {
            break;
        }
    }
    if ((size | (1u << i)) == size) {
        return i;
    }
    return i + 1;
}