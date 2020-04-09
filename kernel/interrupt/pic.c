//
// Created by dxy on 2020/4/2.
//

#include <stdint.h>
#include "pic.h"
#include "../lib/asm_wrapper.h"

void pic_init() {
    // set up master
    outb(M_PIC_CMD, ICW1);
    outb(M_PIC_DATA, M_ICW2);
    outb(M_PIC_DATA, M_ICW3);
    outb(M_PIC_DATA, ICW4);

    // set up slave
    outb(S_PIC_CMD, ICW1);
    outb(S_PIC_DATA, S_ICW2);
    outb(S_PIC_DATA, S_ICW3);
    // NB Automatic EOI mode doesn't tend to work on the slave.
    // Linux source code says it's "to be investigated".
    outb(S_PIC_DATA, ICW4);

    // use non-specific EOI by setting OCW2
    outb(M_PIC_CMD, 0x20u);
    outb(S_PIC_CMD, 0x20u);
    // OCW3:  0ef01prs
    //   ef:  0x = NOP, 10 = clear specific mask, 11 = set specific mask
    //    p:  0 = no polling, 1 = polling mode
    //   rs:  0x = NOP, 10 = read IRR, 11 = read ISR
    outb(M_PIC_CMD, 0x48u); // clear specific mask
    outb(M_PIC_CMD, 0x0au); // read IRR by default

    outb(S_PIC_CMD, 0x48u); // OCW3
    outb(S_PIC_CMD, 0x0au); // OCW3

    // mask all interrupts except IRQ2, so slave PIC will not be disabled
    outb(M_PIC_DATA, 0xfbu);
    outb(S_PIC_DATA, 0xffu);
}

void send_eoi(uint16_t intr_vec) {
    if (intr_vec >= S_ICW2)
        outb(S_PIC_CMD, PIC_EOI);

    outb(M_PIC_CMD, PIC_EOI);
}

void enable_irq(uint16_t intr_vec) {
    uint8_t imr;
    if (intr_vec < S_ICW2) {
        imr = inb(M_PIC_DATA);
        imr &= ~(1u << (intr_vec - M_ICW2));
        outb(M_PIC_DATA, imr);
    } else {
        imr = inb(S_PIC_DATA);
        imr &= ~(1u << (intr_vec - S_ICW2));
        outb(S_PIC_DATA, imr);
    }
}

void disable_irq(uint16_t intr_vec) {
    uint8_t imr;
    if (intr_vec < S_ICW2) {
        imr = inb(M_PIC_DATA);
        imr |= (1u << (intr_vec - M_ICW2));
        outb(M_PIC_DATA, imr);
    } else {
        imr = inb(S_PIC_DATA);
        imr |= (1u << (intr_vec - S_ICW2));
        outb(S_PIC_DATA, imr);
    }
}

void enable_all_irqs() {
    outb(M_PIC_DATA, 0x0u);
    outb(S_PIC_DATA, 0x0u);
}