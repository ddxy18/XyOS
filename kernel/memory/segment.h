//
// Created by dxy on 2020/3/16.
//

#ifndef XYOS_SEGMENT_H
#define XYOS_SEGMENT_H

// segment selector
#define kKCodeSeg 0x8
#define kKDataSeg 0x10
#define kUCodeSeg 0x1b
#define kUDataSeg 0x23

#ifndef XYOS_INTR_ENTRY_H
#define XYOS_INTR_ENTRY_H

#include <stdint.h>

// Tss
static const uintptr_t kTssBaseAddr = 0xc001e000;

typedef struct SegDescriptor {
    // low 32 bits
    uint16_t limit_0_15;
    uint16_t base_addr_0_15;

    // high 32 bits
    uint8_t base_addr_16_23;
    // 4 bits type field
    // descriptor type flag
    // 2 bits descriptor privilege level field
    // SEG-present flag
    uint8_t access;
    uint8_t limit_16_19: 4;
    // available and reserved bits
    // L (64-bit code SEG) flag
    // D/B (default operation size/default stack pointer size and/or upper bound)
    // flag granularity flag
    uint8_t flags: 4;
    uint8_t base_addr_24_31;
} __attribute__((packed)) SegDescriptor;

/**
 * structure of register Gdtr, which should contain information of GDT
 */
typedef struct Gdtr {
    uint16_t limit;
    uint32_t base_addr;
} __attribute__((packed)) Gdtr;

void GdtInit();

#endif //XYOS_INTR_ENTRY_H

#endif //XYOS_SEGMENT_H