//
// Created by dxy on 2020/3/16.
//

#ifndef XYOS_SEGMENT_H
#define XYOS_SEGMENT_H

/**
 * virtual address of GDT
 * should be aligned on an eight-byte boundary to yield the best processor
 * performance
 */
#define GDT_BASE_ADDR 0xc0000000u
// offset of the last descriptor expressed in bytes
#define GDT_LIMIT (GDT_NUM << 3u) - 1u

// GDT has 5 entries
#define GDT_ENTRY_SIZE 8u
#define GDT_NUM 6u
#define GDT_SIZE GDT_NUM *GDT_ENTRY_SIZE

// TSS
#define TSS_BASE_ADDR 0xc001e000u
#define TSS_SIZE 103u
#define TSS_ACCESS 0x89u
#define TSS_FLAGS 0x1u

// segment selector
#define K_CODE_SEG 0x8u
#define K_DATA_SEG 0x10u
#define U_CODE_SEG 0x1bu
#define U_DATA_SEG 0x23u
#define TSS_SEL 0x28u

// each segment has a size of 4 GB
#define SEG_SIZE 0xffffffffu
#define DESCRIPTOR_FLAGS 0xcu
#define SEG_BASE_ADDR 0x0u
#define SEG_PRIORITY_0 0x9u
#define SEG_PRIORITY_3 0xfu
// read/write, accessed
#define SEG_TYPE_DATA 0x2u
// execute/read, conforming, accessed
#define SEG_TYPE_CODE 0xau

#define access(priority, type) (priority << 4u) | type

#ifndef XYOS_INTR_ENTRY_H
#define XYOS_INTR_ENTRY_H

#include <stdint.h>

typedef struct seg_descriptor {
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
} __attribute__((packed)) seg_descriptor_t;

/**
 * structure of register GDTR, which should contain information of GDT
 */
typedef struct GDTR {
    uint16_t limit;
    uint32_t base_addr;
} __attribute__((packed)) GDTR_t;

void gdt_init();

#endif // XYOS_INTR_ENTRY_H

#endif // XYOS_SEGMENT_H