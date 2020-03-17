//
// Created by dxy on 2020/3/16.
//

/**
 * The kernel takes protected flat model as Linux
 * which only help paging to provide isolation between user and supervisor code and data.
 * Some descriptor properties are set to constants as follow:
 * flags  0x3
 * limit  0xffffffff
 * base_address 0x0
 * All four segments begin at 0x0 and has a size of 4 GB, so the logical address is equal to the linear address.
 * As required by the hardware, the first entry in GDT must be set as a null descriptor.
 */

#include "segment.h"

GDTR gdtr;
// gdt points to the null descriptor of the GDT
segment_descriptor_t *gdt;

void set_descriptor(segment_descriptor_t *segment_descriptor, uint32_t base_address, uint32_t limit, uint8_t access,
                    uint8_t flags);

void gdt_init() {
    // determine gdt's physical address and set register GDTR
    gdt = (segment_descriptor_t *) GDT_BASE_ADDRESS;
    gdtr.limit = GDT_LIMIT;
    gdtr.base_address = GDT_BASE_ADDRESS;
    asm volatile ("lgdt %0"
    :
    :"m" (gdtr)
    );
    // set null descriptor
    set_descriptor(&gdt[0], 0u, 0u, 0u, 0u);
    // data segment
    set_descriptor(&gdt[1], SEGMENT_BASE_ADDRESS, SEGMENT_SIZE, access(SEGMENT_PRIORITY_0, SEGMENT_TYPE_DATA),
                   DESCRIPTOR_FLAGS);
    // code segment
    set_descriptor(&gdt[2], SEGMENT_BASE_ADDRESS, SEGMENT_SIZE, access(SEGMENT_PRIORITY_0, SEGMENT_TYPE_CODE),
                   DESCRIPTOR_FLAGS);
    // user data segment
    set_descriptor(&gdt[3], SEGMENT_BASE_ADDRESS, SEGMENT_SIZE, access(SEGMENT_PRIORITY_3, SEGMENT_TYPE_DATA),
                   DESCRIPTOR_FLAGS);
    // user code segment
    set_descriptor(&gdt[4], SEGMENT_BASE_ADDRESS, SEGMENT_SIZE, access(SEGMENT_PRIORITY_3, SEGMENT_TYPE_CODE),
                   DESCRIPTOR_FLAGS);
}

void set_descriptor(segment_descriptor_t *segment_descriptor, uint32_t base_address, uint32_t limit, uint8_t access,
                    uint8_t flags) {
    segment_descriptor->limit_0_15 = limit;
    segment_descriptor->limit_16_19 = limit >> 16u;

    segment_descriptor->base_address_0_15 = base_address;
    segment_descriptor->base_address_16_23 = base_address >> 16u;
    segment_descriptor->base_address_24_31 = base_address >> 24u;

    segment_descriptor->access = access;
    segment_descriptor->flags = flags;
}