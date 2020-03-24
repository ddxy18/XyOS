//
// Created by dxy on 2020/3/22.
//

#include <stddef.h>

#include "process.h"
#include "../lib/list.h"
#include "../lib/bitmap.h"
#include "../memory/direct_mapping_allocator.h"

uint64_t pid_bitmap[PROCESS_MAX_NUM / sizeof(uint64_t)];

linked_list_t process_wait_list;
linked_list_t process_block_list;

void process_manager_init() {
    for (uint32_t i = 0; i < PROCESS_MAX_NUM; ++i) {
        clear_bit(pid_bitmap, i);
    }

    process_wait_list.head = (linked_list_node_t *) request_bytes(sizeof(linked_list_node_t *));
    process_wait_list.head->data = 0;
    process_wait_list.head->next = NULL;
    process_block_list.head = (linked_list_node_t *) request_bytes(sizeof(linked_list_node_t *));
    process_block_list.head->data = 0;
    process_block_list.head->next = NULL;
}

void create_process() {
    // choose a free pid
    process_control_block_t *pcb;
    pcb = (process_control_block_t *) request_bytes(sizeof(process_control_block_t));

    for (uint32_t i = 0; i < PROCESS_MAX_NUM; ++i) {
        if (is_bit_set(pid_bitmap, i) == 0) {
            pcb->pid = i + 1;
        }
    }
    // set page directory
    pcb->page_directory = (page_directory_entry_t *) request_bytes(PAGE_DIRECTORY_SIZE);
    u_page_directory_init(pcb->page_directory);

    // allocate kernel space stack
    pcb->k_stack.virtual_addr = request_bytes(K_STACK_SIZE);
    pcb->k_stack.size = K_STACK_SIZE;
    // allocate user space stack
    pcb->u_stack.virtual_addr = K_VIRTUAL_ADDR - 1;
    pcb->u_stack.size = U_STACK_INIT_SIZE;

    // TODO: determine code and data area's information and state

    // add 'pcb' to 'process_wait_list'
    linked_list_node_t *tmp = (linked_list_node_t *) request_bytes(sizeof(linked_list_node_t *));
    tmp->data = (uint32_t) pcb;
    tmp->next = NULL;
    insert_to_head(process_wait_list, tmp);
}

uint8_t close_process(process_control_block_t *pcb) {
    if (pcb->state == WAIT) {
        remove(process_wait_list, (linked_list_node_t *) pcb);
        clear_bit(pid_bitmap, pcb->pid - 1);
        release_bytes(pcb->k_stack.virtual_addr);

        // release physical pages and corresponding page tables and page directory
        page_directory_entry_t *page_directory = pcb->page_directory;
        for (uint32_t i = 0; i < U_PDE_NUM; ++i) {
            if (page_directory[i].page_table_addr != PDE_ABSENT_ADDR) {
                u_release_pages((page_table_entry_t *) page_directory[i].page_table_addr);
                release_bytes(direct_mapping_virtual_addr(page_directory[i].page_table_addr));
            }
        }
        release_bytes((uint32_t) page_directory);
        return 0;
    } else {
        // process in block state will wait until it changes to wait state.
        return 1;
    }
}