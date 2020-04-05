//
// Created by dxy on 2020/3/28.
//

#ifndef XYOS_MEMORY_MANAGER_H
#define XYOS_MEMORY_MANAGER_H

#include "../lib/def.h"

#define AVL_MEM_ADDR 0x20000U

typedef struct avl_phys_mem {
  uintptr_t addr;
    uint32_t size;
} avl_phys_mem_t;

/**
 * @brief Get the 'avl_mem'. 'start.c' will use it to get 'avl_mem'
 * and set available memory information.
 *
 * @return avl_phys_mem_t* Return address of 'avl_mem'.
 */
avl_phys_mem_t *get_avl_mem();

/**
 * @brief Open paging, reset GDT and initialize page allocator.
 */
void mem_init();

#endif // XYOS_MEMORY_MANAGER_H