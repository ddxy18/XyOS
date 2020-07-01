//
// Created by dxy on 2020/3/28.
//

#ifndef XYOS_MEMORY_MANAGER_H
#define XYOS_MEMORY_MANAGER_H

#include "../lib/def.h"

typedef struct AvlPhysMem {
    uintptr_t addr;
    uint32_t size;
} AvlPhysMem;

/**
 * @brief Get the 'avl_mem'.
 *
 * 'start.c' will use it to get 'avl_mem' and set available memory information.
 *
 * @return AvlPhysMem * Return the pointer of 'avl_mem'.
 */
AvlPhysMem *GetAvlMem();

/**
 * @brief Open paging, reset GDT and initialize page allocator.
 */
void MemInit();

#endif //XYOS_MEMORY_MANAGER_H