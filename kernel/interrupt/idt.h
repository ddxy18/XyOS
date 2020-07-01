//
// Created by dxy on 2020/3/24.
//

#ifndef XYOS_IDT_H
#define XYOS_IDT_H

#include <stdint.h>

/**
 * vector numbers range from 0 to 255
 * 0 through 31 reserved by the Intel 64 and IA-32 architectures for
 * architecture-defined exceptions and interrupts 32 to 255 designated as
 * user-defined interrupts
 */
#define kIdtEntryNum 256

// Intel predetermined 0~17 for NMI and exceptions.
static const int kIdtExcNum = 32;

// TSS
typedef struct Tss {
    uint32_t prev_task_link;  // old ts selector
    uint32_t esp0;
    uint32_t ss0;
    uint32_t esp1;
    uint32_t ss1;
    uint32_t esp2;
    uint32_t ss2;
    uint32_t cr3;
    uint32_t eip;
    uint32_t eflags;
    uint32_t eax;
    uint32_t ecx;
    uint32_t edx;
    uint32_t ebx;
    uint32_t esp;
    uint32_t ebp;
    uint32_t esi;
    uint32_t edi;
    uint32_t es;
    uint32_t cs;
    uint32_t ss;
    uint32_t ds;
    uint32_t fs;
    uint32_t gs;
    uint32_t ldt_seg_sel;
    // When set, switch to this task will generate a debug exception.
    uint16_t t;
    uint16_t io_map_addr; // i/o map base address
    uint32_t ssp;
} __attribute__((packed)) Tss;

typedef struct IntrReg {
    uint16_t ds;
    uint16_t es;
    uint16_t fs;
    uint16_t gs;
    // '%edi' to '%eax' are pushed in stack by 'pushal'
    uint32_t p_edi;
    uint32_t p_esi;
    uint32_t p_ebp;
    uint32_t p_esp;
    uint32_t p_ebx;
    uint32_t p_edx;
    uint32_t p_ecx;
    uint32_t p_eax;
    // interrupt vector number
    uint32_t intr_vec_no;
    // All below are pushed by processor.
    /**
     * 'err_code' is pushed by OS when it doesn't exist to satisfy common
     * structure. Moreover, all 'err_code' should be popped by OS.
     */
    uint32_t err_code;
    uint32_t eip;
    uint32_t cs;
    uint32_t eflags;
    // pushed when privilege-level changes
    uint32_t esp;
    uint16_t ss;
} __attribute__((packed)) IntrReg;

typedef struct TaskGateDescriptor {
    uint16_t reserved_0_15;
    uint16_t tss_seg_sel;
    uint16_t type;
    uint16_t reserved_24_39;
} __attribute__((packed)) TaskGateDescriptor;

typedef struct IntrGateDescriptor {
    uint16_t offset_0_15;
    uint16_t seg_sel;
    uint16_t type;
    uint16_t offset_16_31;
} __attribute__((packed)) IntrGateDescriptor;

typedef struct TrapGateDescriptor {
    uint16_t offset_0_15;
    uint16_t seg_sel;
    uint16_t type;
    uint16_t offset_16_31;
} __attribute__((packed)) TrapGateDescriptor;

typedef struct IDTR {
    uint16_t limit;
    uint32_t base_addr;
} __attribute__((packed)) Idtr;

Tss *tss;

void IdtInit();

/**
 * Set IDT entry's present flag.
 *
 * @param intr_vec_no
 */
void EnableIdt(int intr_vec_no);

void DisableIdt(int intr_vec_no);

#endif // XYOS_IDT_H
