//
// Created by dxy on 2020/3/20.
//

#ifndef XYOS_BITMAP_H
#define XYOS_BITMAP_H

#include <stdint.h>
#include "def.h"

void bitmap_set(uint64_t *bitmap, uint32_t i);

void bitmap_clear(uint64_t *bitmap, uint32_t i);

/**
 * @brief Determine whether bit 'i' is set.
 *
 * @param bitmap
 * @param i
 * @return bool
 */
bool is_bitmap_set(uint64_t *bitmap, uint32_t i);

#endif // XYOS_BITMAP_H
