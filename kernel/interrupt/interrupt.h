//
// Created by dxy on 2020/4/2.
//

#ifndef XYOS_INTERRUPT_H
#define XYOS_INTERRUPT_H

#include "../lib/def.h"
#include "idt.h"

// interrupt vector numbers
#define PF_VEC 14u
#define TIMER_VEC 32u
#define KEYBOARD_VEC 33u
#define IDE_VEC 46u

void intr_init();

bool reg_intr_handler(uint16_t intr_vec, void (*handler)(intr_reg_t *));

void remove_intr_handler(uint16_t intr_vec_no);

#endif // XYOS_INTERRUPT_H