//
// Created by dxy on 2020/3/20.
//

#ifndef XYOS_BITMAP_H
#define XYOS_BITMAP_H

#include "def.h"

/**
 * Initialize all bit to 'state'.
 *
 * @param bitmap
 * @param arr_size
 * @param state
 */
static inline void Bitmap(unsigned int *bitmap, int arr_size, bool state) {
    for (int i = 0; i < arr_size; ++i) {
        bitmap[i] = 0;
        if (state == TRUE) {
            bitmap[i] = ~bitmap[i];
        }
    }
}

static inline void SetBitmap(unsigned int *bitmap, int i) {
    uint32_t arr_i = i / (sizeof(unsigned int) * 8);
    // find the bit location in bitmap[arr_i]
    i %= sizeof(unsigned int);
    bitmap[arr_i] |= (1llu << i);
}

static inline void ClearBitmap(unsigned int *bitmap, int i) {
    uint32_t arr_i = i / (sizeof(unsigned int) * 8);
    i %= sizeof(unsigned int);
    bitmap[arr_i] &= (~(1llu << i));
}

static inline bool IsBitmapSet(const unsigned int *bitmap, int i) {
    uint32_t arr_i = i / (sizeof(unsigned int) * 8);
    i %= sizeof(unsigned int);
    return (bitmap[arr_i] & (1llu << i)) != 0;
}

#endif // XYOS_BITMAP_H