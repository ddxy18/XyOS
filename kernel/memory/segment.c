//
// Created by dxy on 2020/3/16.
//

/**
 * The kernel takes protected flat model which only help paging to provide
 * isolation between user and supervisor code and data. Some descriptor
 * properties are set to constants as follow:
 * flags 0x3
 * limit 0xffffffff
 * base_addr 0x0
 * All four segments begin at 0x0 and has a size of 4 GB, so the logical
 * address is equal to the virtual address. As required by the hardware, the
 * first entry in GDT must be set as a null descriptor. Moreover, we have
 * already open paging, so we will use virtual addresses.
 */

#include "segment.h"

#include "../driver/console.h"

/**
 * virtual address of GDT
 * It should be aligned on an eight-byte boundary to yield the best processor
 * performance.
 */
static const uintptr_t kGdtBaseAddr = 0xc0000000;

// GDT has 5 entries
static const int kGdtEntrySize = 8;
static const int kGdtNum = 6;
static const int kGdtSize = kGdtNum * kGdtEntrySize;

// each segment has a size of 4 GB
static const unsigned int kSegSize = 0xffffffff;
static const int kDescriptorFlags = 0xc;
static const uintptr_t kSegBaseAddr = 0;
static const int kSegPriority0 = 0x9;
static const int kSegPriority3 = 0xf;
// read/write, accessed
static const int kSegTypeData = 0x2;
// execute/read, conforming, accessed
static const int kSegTypeCode = 0xa;

// Tss
static const int sTssSize = 103;
static const int kTssAccess = 0x89;
static const int kTssFlags = 0x1;

static inline int Access(int priority, int type) {
    return (int) (((unsigned) priority << 4u) | (unsigned) type);
}

// offset of the last descriptor expressed in bytes
static inline int GdtLimit() {
    return (int) ((unsigned) kGdtNum << 3u) - 1;
}

static void set_gdt_entry(SegDescriptor *seg_descriptor, uint32_t base_addr,
                          uint32_t limit, uint8_t access, uint8_t flags);

//  Test whether segment register value is as expected.
void print_seg_reg();

void GdtInit() {
    // move gdt to appointed address
    SegDescriptor *gdt = (SegDescriptor *) kGdtBaseAddr;

    Gdtr gdtr;
    gdtr.limit = (uint16_t) GdtLimit;
    gdtr.base_addr = (uint32_t) gdt;

    // use 'gdt[0]' as a null descriptor
    set_gdt_entry(&gdt[0], 0, 0, 0, 0);
    // kernel code segment
    set_gdt_entry(&gdt[1], kSegBaseAddr, kSegSize,
                  Access(kSegPriority0, kSegTypeCode), kDescriptorFlags);
    // kernel data segment
    set_gdt_entry(&gdt[2], kSegBaseAddr, kSegSize,
                  Access(kSegPriority0, kSegTypeData), kDescriptorFlags);
    // user code segment
    set_gdt_entry(&gdt[3], kSegBaseAddr, kSegSize,
                  Access(kSegPriority3, kSegTypeCode), kDescriptorFlags);
    // user data segment
    set_gdt_entry(&gdt[4], kSegBaseAddr, kSegSize,
                  Access(kSegPriority3, kSegTypeData), kDescriptorFlags);
    // set a Tss for one CPU
    set_gdt_entry(&gdt[5], kTssBaseAddr, sTssSize, kTssAccess,
                  kTssFlags);

    // let '%gdtr' point to new GDT
    asm("lgdt %0" : : "m"(gdtr));

    // update segment registers
    asm("mov %0, %%eax"::"r"(kKDataSeg));
    asm("mov %ax, %ds");
    asm("mov %ax, %es");
    asm("mov %ax, %ss");
//    asm("mov %0, %%eax"::"r"(kUDataSeg));
    asm("mov %ax, %fs");
    asm("mov %ax, %gs");
}

static void set_gdt_entry(SegDescriptor *seg_descriptor, uint32_t base_addr,
                          uint32_t limit, uint8_t access, uint8_t flags) {
    seg_descriptor->limit_0_15 = limit;
    seg_descriptor->limit_16_19 = limit >> 16u;

    seg_descriptor->base_addr_0_15 = base_addr;
    seg_descriptor->base_addr_16_23 = base_addr >> 16u;
    seg_descriptor->base_addr_24_31 = base_addr >> 24u;

    seg_descriptor->access = access;
    seg_descriptor->flags = flags;
}

void print_seg_reg() {
    // TODO: allocate memory for buf
    char *buf;
    uint16_t cs;
    asm volatile("mov %%cs, %0"::"m"(cs) : "memory");
    uint32_t c = cs;
    itoa(buf, 'x', c);
    printf("cs=");
    printf(buf);
    printf(" ");
    uint16_t ds;
    asm volatile("mov %%ds, %0"::"m"(ds) : "memory");
    uint32_t d = ds;
    itoa(buf, 'x', d);
    printf("ds=");
    printf(buf);
    printf(" ");
    uint16_t es;
    asm volatile("mov %%es, %0"::"m"(es) : "memory");
    uint32_t e = es;
    itoa(buf, 'x', e);
    printf("es=");
    printf(buf);
    printf(" ");
    uint16_t fs;
    asm volatile("mov %%fs, %0"::"m"(fs) : "memory");
    uint32_t f = fs;
    itoa(buf, 'x', f);
    printf("fs=");
    printf(buf);
    printf(" ");
    uint16_t gs;
    asm volatile("mov %%gs, %0"::"m"(gs) : "memory");
    uint32_t g = gs;
    itoa(buf, 'x', g);
    printf("gs=");
    printf(buf);
    printf(" ");
    uint16_t ss;
    asm volatile("mov %%ss, %0"::"m"(ss) : "memory");
    uint32_t s = ss;
    itoa(buf, 'x', s);
    printf("ss=");
    printf(buf);
    printf(" ");

    printf("\n");
}