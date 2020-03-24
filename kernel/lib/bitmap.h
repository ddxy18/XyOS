//
// Created by dxy on 2020/3/20.
//

#ifndef XYOS_BITMAP_H
#define XYOS_BITMAP_H

#include <stdint.h>

void set_bit(uint64_t bitmap[], uint32_t i) {
    uint32_t arr_i = i / 64;
    // find the bit location in bitmap[arr_i]
    i %= 64;
    bitmap[arr_i] |= (1lu << i);
}

void clear_bit(uint64_t bitmap[], uint32_t i) {
    uint32_t arr_i = i / 64;
    // find the bit location in bitmap[arr_i]
    i %= 64;
    bitmap[arr_i] &= (!(1lu << i));
}

// If bit i is set, return 1.Otherwise return 0.
uint8_t is_bit_set(uint64_t bitmap[], uint32_t i) {
    uint32_t arr_i = i / 64;
    // find the bit location in bitmap[arr_i]
    i %= 64;
    return (bitmap[arr_i] & (1lu << i)) != 0;
}

#endif //XYOS_BITMAP_H
