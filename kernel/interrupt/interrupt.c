//
// Created by dxy on 2020/4/2.
//

#include "interrupt.h"

#include <stddef.h>

#include "pic.h"
#include "../lib/bitmap.h"
#include "../driver/console.h"
#include "../memory/paging.h"

static void ExcHandler(IntrReg *intr_reg);

// record whether interrupt vector number is used
static unsigned int IntrVecMap[8];

// core handler that is provided by users
static void (*intr_handler[kIdtEntryNum])(IntrReg *);

void IntrInit() {
    IdtInit();
    PicInit();
    // clear all bits in 'IntrVecMap'
    Bitmap(IntrVecMap, 8, 0);
    // register common handler for Intel predetermined exceptions
    for (int i = 0; i < kIdtExcNum; ++i) {
        RegIntrHandler(i, ExcHandler);
    }
    // replace page fault handler
    RemoveIntrHandler(kPfVec);
    RegIntrHandler(kPfVec, PfHandler);
}

bool RegIntrHandler(int intr_vec_no, void (*handler)(IntrReg *)) {
    if (IsBitmapSet(IntrVecMap, intr_vec_no) == TRUE) {
        return FALSE;
    }
    SetBitmap(IntrVecMap, intr_vec_no);
    intr_handler[intr_vec_no] = handler;
    EnableIdt(intr_vec_no);
    return TRUE;
}

void RemoveIntrHandler(int intr_vec_no) {
    DisableIdt(intr_vec_no);
    intr_handler[intr_vec_no] = NULL;
    ClearBitmap(IntrVecMap, intr_vec_no);
}

static void ExcHandler(IntrReg *intr_reg) {
    uint32_t intr_vec_no = intr_reg->intr_vec_no;
    printf("handling interrupt ");
    char *buf = (char *) 0x100000u;
    itoa(buf, 'd', intr_vec_no);
    printf(buf);
    printf("\n");
    SendEoi(intr_reg->intr_vec_no);
}

void IntrHandlerSel(IntrReg *intr_reg) {
    if (intr_handler[intr_reg->intr_vec_no] != NULL) {
        intr_handler[intr_reg->intr_vec_no](intr_reg);
    }
}