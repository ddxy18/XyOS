//
// Created by dxy on 2020/4/3.
//

#include "bitmap.h"

void bitmap_set(uint64_t *bitmap, uint32_t i) {
    uint32_t arr_i = i / 64;
    // find the bit location in bitmap[arr_i]
    i %= 64;
    bitmap[arr_i] |= (1llu << i);
}

void bitmap_clear(uint64_t *bitmap, uint32_t i) {
    uint32_t arr_i = i / 64;
    // find the bit location in bitmap[arr_i]
    i %= 64;
    bitmap[arr_i] &= (!(1llu << i));
}

// If bit i is set, return 1.Otherwise return 0.
bool is_bitmap_set(uint64_t *bitmap, uint32_t i) {
    uint32_t arr_i = i / 64;
    // find the bit location in bitmap[arr_i]
    i %= 64;
    return (bitmap[arr_i] & (1llu << i)) != 0;
}
