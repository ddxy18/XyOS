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
 * All four segments begin at 0x0 and has a size of 4 GB, so
 * the logical address is equal to the virtual address. As required by the
 * hardware, the first entry in GDT must be set as a null descriptor. Moreover,
 * we have already open paging, so we will use virtual addresses.
 */

#include "segment.h"
#include "../driver/console.h"
#include "direct_mapping_allocator.h"

GDTR_t gdtr;
// gdt points to the null descriptor of the GDT
seg_descriptor_t *gdt;

void set_gdt_entry(seg_descriptor_t *, uint32_t, uint32_t, uint8_t, uint8_t);

//  Test whether segment register value is as expected.
void print_seg_reg();

// void cs_flush() { return; }

void gdt_init() {
  // move gdt to appointed address
  gdt = (seg_descriptor_t *)GDT_BASE_ADDR;

  gdtr.limit = GDT_LIMIT;
  gdtr.base_addr = (uint32_t)gdt;

  // use 'gdt[0]' as a null descriptor
  set_gdt_entry(&gdt[0], 0, 0, 0, 0);
  // kernel code segment
  set_gdt_entry(&gdt[1], SEG_BASE_ADDR, SEG_SIZE,
                access(SEG_PRIORITY_0, SEG_TYPE_CODE), DESCRIPTOR_FLAGS);
  // kernel data segment
  set_gdt_entry(&gdt[2], SEG_BASE_ADDR, SEG_SIZE,
                access(SEG_PRIORITY_0, SEG_TYPE_DATA), DESCRIPTOR_FLAGS);
  // user code segment
  set_gdt_entry(&gdt[3], SEG_BASE_ADDR, SEG_SIZE,
                access(SEG_PRIORITY_3, SEG_TYPE_CODE), DESCRIPTOR_FLAGS);
  // user data segment
  set_gdt_entry(&gdt[4], SEG_BASE_ADDR, SEG_SIZE,
                access(SEG_PRIORITY_3, SEG_TYPE_DATA), DESCRIPTOR_FLAGS);
  // set a TSS for one CPU
  set_gdt_entry(&gdt[5], TSS_BASE_ADDR, TSS_SIZE, TSS_ACCESS, TSS_FLAGS);

  // let '%gdtr' point to new GDT
  asm("lgdt %0" : : "m"(gdtr));

  // update segment registers
  asm("mov %0, %%eax" ::"r"(K_DATA_SEG));
  asm("mov %ax, %ds");
  asm("mov %ax, %es");
  asm("mov %ax, %ss");
  // asm("mov %0, %%eax"::"r"(U_DATA_SEG));
  asm("mov %ax, %fs");
  asm("mov %ax, %gs");
  // asm("jmp 0x08:_cs_flush");
}

void set_gdt_entry(seg_descriptor_t *seg_descriptor, uint32_t base_addr,
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
  char *buf;
  uint16_t cs;
  asm volatile("mov %%cs, %0" ::"m"(cs) : "memory");
  uint32_t c = cs;
  itoa(buf, 'x', c);
  printf("cs=");
  printf(buf);
  printf(" ");
  uint16_t ds;
  asm volatile("mov %%ds, %0" ::"m"(ds) : "memory");
  uint32_t d = ds;
  itoa(buf, 'x', d);
  printf("ds=");
  printf(buf);
  printf(" ");
  uint16_t es;
  asm volatile("mov %%es, %0" ::"m"(es) : "memory");
  uint32_t e = es;
  itoa(buf, 'x', e);
  printf("es=");
  printf(buf);
  printf(" ");
  uint16_t fs;
  asm volatile("mov %%fs, %0" ::"m"(fs) : "memory");
  uint32_t f = fs;
  itoa(buf, 'x', f);
  printf("fs=");
  printf(buf);
  printf(" ");
  uint16_t gs;
  asm volatile("mov %%gs, %0" ::"m"(gs) : "memory");
  uint32_t g = gs;
  itoa(buf, 'x', g);
  printf("gs=");
  printf(buf);
  printf(" ");
  uint16_t ss;
  asm volatile("mov %%ss, %0" ::"m"(ss) : "memory");
  uint32_t s = ss;
  itoa(buf, 'x', s);
  printf("ss=");
  printf(buf);
  printf(" ");

  printf("\n");
}