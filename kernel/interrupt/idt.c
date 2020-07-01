//
// Created by dxy on 2020/3/24.
//

#include "idt.h"

#include "../memory/segment.h"

static const int kIdtEntrySize = 8;
static const int kIdtSize = kIdtEntryNum * kIdtEntrySize;
// The base addresses of the IDT should be aligned on an 8-byte boundary to
// maximize performance of cache line fills.
static const uintptr_t kIdtBaseAddr = 0xc001f000;

// present flag is cleared
static const int kIdtExcType = 0x0e00;
static const int kIdtIntrType = 0x0e00;
static const int kIdtTrapType = 0x6f00;
static const int kIdtSyscallNum = 0x80;

static const int kTssSel = 0x28;

static uint64_t *const idt = (uint64_t *const) kIdtBaseAddr;
static Idtr idtr;
extern uintptr_t intr_vec[];

static const int IdtLimit = ((unsigned) kIdtEntryNum << 3u) - 1;

static void SetIdtEntry(int intr_vec_no, uint16_t seg_sel, uint16_t
type);

void IdtInit() {
    idtr.limit = (unsigned) IdtLimit;
    idtr.base_addr = kIdtBaseAddr;
    asm volatile("lidt (%0)"::"r"(&idtr) : "memory");

    /**
     * Initialize the only TSS. Only use 'ss0' and 'esp0' to help jump to
     * ring0 when generating interrupts in ring3. Every thread should set its
     * own 'esp0' when it's running.
     */
    tss = (Tss *) kTssBaseAddr;
    tss->ss0 = kKCodeSeg;

    // set all 256 vector numbers handler entries to a prehandler
    int i = 0;
    // initialize Intel predetermined exceptions
    for (; i < kIdtExcNum; ++i) {
        SetIdtEntry(i, kKCodeSeg, kIdtExcType);
    }
    // initialize external interrupts
    for (; i < kIdtEntryNum; ++i) {
        SetIdtEntry(i, kKCodeSeg, kIdtIntrType);
    }
    // TODO: allow ring3 to use 'int 0x80'
    SetIdtEntry(kIdtSyscallNum, kUCodeSeg, kIdtTrapType);

    // Set task register. It will never be changed since now.
    uint16_t tr = kTssSel;
    asm("ltr %0" : : "r"(tr));
}

static void SetIdtEntry(int intr_vec_no, uint16_t seg_sel, uint16_t type) {
    IntrGateDescriptor *intr_entry = (IntrGateDescriptor *) &idt[intr_vec_no];
    intr_entry->offset_0_15 = intr_vec[intr_vec_no];
    intr_entry->offset_16_31 = (intr_vec[intr_vec_no] >> 16u);
    intr_entry->seg_sel = seg_sel;
    intr_entry->type = type;
}

void EnableIdt(int intr_vec_no) {
    IntrGateDescriptor *intr_entry = (IntrGateDescriptor *) &idt[intr_vec_no];
    intr_entry->type |= 0x8000u;
}

void DisableIdt(int intr_vec_no) {
    IntrGateDescriptor *intr_entry = (IntrGateDescriptor *) &idt[intr_vec_no];
    intr_entry->type &= 0x7fffu;
}