//
// Created by dxy on 2020/3/22.
//

#include <stddef.h>

#include "process.h"
#include "../lib/list.h"
#include "../lib/bitmap.h"
#include "../memory/direct_mapping_allocator.h"

uint64_t pid_bitmap[PROCESS_MAX_NUM / sizeof(uint64_t)];

linked_list_t process_list;

void process_manager_init() {
    for (uint32_t i = 0; i < PROCESS_MAX_NUM; ++i) {
        clear_bit(pid_bitmap, i);
    }

    process_list.head = (linked_list_node_t *) request_bytes(sizeof(linked_list_node_t *));
    process_list.head->data = 0;
    process_list.head->next = NULL;
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

    // add 'pcb' to 'process_list'
    linked_list_node_t *tmp = (linked_list_node_t *) request_bytes(sizeof(linked_list_node_t *));
    tmp->data = (uint32_t) pcb;
    tmp->next = NULL;
    insert_to_head(process_list, tmp);
}
