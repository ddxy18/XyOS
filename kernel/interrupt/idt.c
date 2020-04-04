//
// Created by dxy on 2020/3/24.
//

#include "idt.h"
#include "../memory/direct_mapping_allocator.h"
#include "../memory/segment.h"

void set_idt_entry(uint16_t, uint16_t, uint16_t);

uint64_t *idt;
IDTR_t idtr;
extern pointer_t intr_vec[];

void idt_init() {
    idtr.limit = IDT_LIMIT;
    idtr.base_addr = IDT_BASE_ADDR;
    idt = (uint64_t *) idtr.base_addr;

    // Initialize the only TSS. Only use 'ss0' and 'esp0' to help jump to ring0 when generating interrupts in ring3.
    // Every process should set its own 'esp0' when it's running.
    tss = (TSS_t *) TSS_BASE_ADDR;
    tss->ss0 = K_CODE_SEG;
    // TODO: Every process should set esp0 when it's turn to execute.

    // set all 256 vector numbers handler entries to a prehandler
    uint32_t i = 0;
    // initialize Intel predetermined exceptions
    for (; i < IDT_EXC_NUM; ++i) {
        set_idt_entry(i, K_CODE_SEG, IDT_EXC_TYPE);
    }
    // initialize external interrupts
    for (; i < IDT_ENTRY_NUM; ++i) {
        set_idt_entry(i, K_CODE_SEG, IDT_INTR_TYPE);
    }
    // TODO: allow ring3 to use 'int 0x80'
    set_idt_entry(IDT_SYSCALL_NUM, U_CODE_SEG, IDT_TRAP_TYPE);

    // Set task register. It will never be changed since now.
    uint16_t tr = TSS_SEL;
    asm("ltr %0" : : "r"(tr));

    asm volatile("lidt (%0)"::"r"(&idtr) : "memory");
}

void set_idt_entry(uint16_t intr_vec_no, uint16_t seg_sel, uint16_t type) {
    intr_gate_descriptor_t *intr_entry = (intr_gate_descriptor_t *) &idt[intr_vec_no];
    intr_entry->offset_0_15 = intr_vec[intr_vec_no];
    intr_entry->offset_16_31 = (intr_vec[intr_vec_no] >> 16u);
    intr_entry->seg_sel = seg_sel;
    intr_entry->type = type;
}

void enable_idt(uint16_t intr_vec_no) {
    intr_gate_descriptor_t *intr_entry = (intr_gate_descriptor_t *) &idt[intr_vec_no];
    intr_entry->type |= 0x8000u;
}

void disable_idt(uint16_t intr_vec_no) {
    intr_gate_descriptor_t *intr_entry = (intr_gate_descriptor_t *) &idt[intr_vec_no];
    intr_entry->type &= 0x7fffu;
}