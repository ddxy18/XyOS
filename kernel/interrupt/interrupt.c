//
// Created by dxy on 2020/4/2.
//

#include <stddef.h>

#include "interrupt.h"
#include "pic.h"
#include "../lib/bitmap.h"
#include "../driver/console.h"
#include "../memory/paging.h"

void exc_handler(intr_reg_t *);

// record whether interrupt vector number is used
uint64_t intr_vec_map[4];

// core handler that is provided by users
void (*intr_handler[IDT_ENTRY_NUM])(intr_reg_t *);

void intr_init() {
    idt_init();
    pic_init();
    // clear all bits in 'intr_vec_map'
    for (uint8_t i = 0; i < 4; ++i) {
        intr_vec_map[i] = 0;
    }
    // register common handler for Intel predetermined exceptions
    for (uint32_t i = 0; i < IDT_EXC_NUM; ++i) {
        reg_intr_handler(i, exc_handler);
    }
    // set page fault handler
    remove_intr_handler(PF_VEC);
    reg_intr_handler(PF_VEC, pf_handler);
}

bool reg_intr_handler(uint16_t intr_vec_no, void (*handler)(intr_reg_t *)) {
    if (is_bitmap_set(intr_vec_map, intr_vec_no) == TRUE) {
        return FALSE;
    }
    bitmap_set(intr_vec_map, intr_vec_no);
    intr_handler[intr_vec_no] = handler;
    enable_idt(intr_vec_no);
    return TRUE;
}

void remove_intr_handler(uint16_t intr_vec_no) {
    disable_idt(intr_vec_no);
    intr_handler[intr_vec_no] = NULL;
    bitmap_clear(intr_vec_map, intr_vec_no);
}

void exc_handler(intr_reg_t *intr_reg) {
    uint32_t intr_vec_no = intr_reg->intr_vec_no;
    printf("handling interrupt ");
    char *buf = (char *) 0x100000u;
    itoa(buf, 'd', intr_vec_no);
    printf(buf);
    printf("\n");
    send_eoi(intr_reg->intr_vec_no);
}

void intr_handler_sel(intr_reg_t *intr_reg) {
    if (intr_handler[intr_reg->intr_vec_no] != NULL) {
        intr_handler[intr_reg->intr_vec_no](intr_reg);
    }
}