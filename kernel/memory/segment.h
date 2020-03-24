//
// Created by dxy on 2020/3/16.
//

#ifndef XYOS_SEGMENT_H
#define XYOS_SEGMENT_H

#include <stdint.h>

/**
 * physical address of GDT
 * should be aligned on an eight-byte boundary to yield the best processor performance
 */
#define GDT_BASE_ADDR 0x1000u
// offset of the last descriptor expressed in bytes
#define GDT_LIMIT (GDT_NUM<<3u)-1u

// GDT has 5 entries
#define GDT_ENTRY_SIZE 8u
#define GDT_NUM 5u
#define GDT_SIZE GDT_NUM * GDT_ENTRY_SIZE

// segment selector
#define K_DATA_SEGMENT 0x8u
#define K_CODE_SEGMENT 0x10u
#define U_DATA_SEGMENT 0x1bu
#define U_CODE_SEGMENT 0x23u

// each segment has a size of 4 GB
#define SEGMENT_SIZE 0xffffffffu
#define DESCRIPTOR_FLAGS 0x3u
#define SEGMENT_BASE_ADDR 0x0u
#define SEGMENT_PRIORITY_0 0x9u
#define SEGMENT_PRIORITY_3 0xfu
// read/write, accessed
#define SEGMENT_TYPE_DATA 0x3u
// execute/read, conforming, accessed
#define SEGMENT_TYPE_CODE 0xfu

#define access(priority, type) (priority<<4u)|type

typedef struct segment_descriptor {
    // low 32 bits
    uint16_t limit_0_15;
    uint16_t base_address_0_15;

    // high 32 bits
    uint8_t base_address_16_23;
    // 4 bits type field
    // descriptor type flag
    // 2 bits descriptor privilege level field
    // segment-present flag
    uint8_t access;
    uint8_t limit_16_19: 4;
    // available and reserved bits
    // L (64-bit code segment) flag
    // D/B (default operation size/default stack pointer size and/or upper bound) flag
    // granularity flag
    uint8_t flags: 4;
    uint8_t base_address_24_31;
} segment_descriptor_t;

/**
 * structure of register GDTR, which should contain information of GDT
 */
typedef struct GDTR {
    uint16_t limit;
    uint32_t base_address;
} GDTR_t;

void gdt_init();

#endif //XYOS_SEGMENT_H