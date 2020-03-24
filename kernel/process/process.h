//
// Created by dxy on 2020/3/22.
//

#ifndef XYOS_PROCESS_H
#define XYOS_PROCESS_H

#include <stdint.h>

#include "../memory/paging.h"
#include "../lib/def.h"

#define PROCESS_MAX_NUM 0x10000u

#define K_STACK_SIZE 0x1000u
#define U_STACK_INIT_SIZE 0x1000u

enum process_state {
    BLOCK, WAIT, RUN
};

typedef struct memory_record {
    uint32_t virtual_addr;
    uint32_t size;
} memory_record_t;

typedef struct process_control_block {
    uint32_t pid;
    enum process_state state;
    page_directory_entry_t *page_directory;
    memory_record_t code;
    memory_record_t data;
    memory_record_t k_stack;
    // 'u_stack.virtual_addr' must point to upper address of the allocated slice since user stack grows to lower address
    memory_record_t u_stack;
} process_control_block_t;

void process_manager_init();

void create_process();

void close_process(uint32_t pid);

#endif //XYOS_PROCESS_H
