//
// Created by dxy on 2020/4/5.
//

#ifndef XYOS_BIT_H
#define XYOS_BIT_H

#include "def.h"

static inline bool IsBitSet(int data, int i) {
    return ((unsigned) data & (1u << (unsigned) i)) != 0;
}

static inline void SetBit(int *data, int i) {
    *data = (int) ((unsigned) *data | (1u << (unsigned) i));
}

static inline void ClearBit(int *data, int i) {
    *data = (int) ((unsigned) *data & (~(1u << (unsigned) i)));
}

#endif //XYOS_BIT_H
