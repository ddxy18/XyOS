//
// Created by dxy on 2020/4/5.
//

#ifndef XYOS_BIT_H
#define XYOS_BIT_H

#include "def.h"

static inline bool is_bit_set(uint32_t data, uint8_t i) {
    return (data & (1u << i)) != 0;
}

static inline void set_bit(uint32_t *data, uint8_t i) {
    *data |= (1u << i);
}

static inline void clear_bit(uint32_t *data, uint8_t i) {
    *data &= ~(1u << i);
}

#endif //XYOS_BIT_H
