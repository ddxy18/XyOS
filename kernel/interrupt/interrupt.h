//
// Created by dxy on 2020/4/2.
//

#ifndef XYOS_INTERRUPT_H
#define XYOS_INTERRUPT_H

#include "../lib/def.h"
#include "idt.h"

// interrupt vector numbers
static const int kPfVec = 14;
static const int kTimerVec = 0x20;
static const int kKeyboardVec = 33;
static const int kIdeVec = 46;
static const int kSyscallVec = 0x80;

void IntrInit();

bool RegIntrHandler(int intr_vec_no, void (*handler)(IntrReg *));

void RemoveIntrHandler(int intr_vec_no);

#endif // XYOS_INTERRUPT_H