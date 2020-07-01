//
// Created by dxy on 2020/4/2.
//

#include "pic.h"

#include <stdint.h>

#include "../lib/asm_wrapper.h"

// I/O Addresses of the two programmable interrupt controllers
static const int kMPicCmd = 0x20;
static const int kMPicData = 0x21;
static const int kSPicCmd = 0xa0;
static const int kSPicData = 0xa1;

static const int kIrqSlave = 0x2;

static const int kMIrqNum = 0x8;
static const int kSIrqNum = 0x8;

static const int kIrqNum = kMIrqNum + kSIrqNum;
/**
 * ICW1
 * @verbatim
 * format: 0001c0ba
 * c  0 = edge triggering, 1 = level triggering
 * b  0 = cascaded PICs, 1 = master only
 * a  0 = no kIcw4, 1 = kIcw4 required
 * @endverbatim
 */
static const int kIcw1 = 0x11;
/**
 * ICW2: vector offset
 */
static const int kMIcw2 = 0x20;

static const int kSIcw2 = kMIcw2 + 8;

/**
 * ICW3
 * @verbatim
 * (master PIC) bit mask of IR lines connected to slaves
 * (slave PIC) 3-bit # of slave's connection to master
 * @endverbatim
 */
static int const kMIcw3 = (int) (1u << (unsigned) kIrqSlave);

static const int kSIcw3 = kIrqSlave;
/**
 * ICW4
 * @verbatim
 * format: 000edcba
 * a:  0 = MCS-80/85 mode, 1 = intel x86 mode
 * b:  1 = Automatic EOI mode
 * c:  0 = slave PIC, 1 = master PIC
 *     (ignored when b is 0, as the master/slave role can be hardwired).
 * d:  1 = buffered mode
 * e:  1 = special fully nested mode
 * @endverbatim
 */
static const int kIcw4 = 0x1;

static const int kPicEoi = 0x20;
static const int kPicReadIrr = 0xa;
static const int kPicReadTsr = 0xb;

void PicInit() {
    // set up master
    outb(kMPicCmd, kIcw1);
    outb(kMPicData, kMIcw2);
    outb(kMPicData, kMIcw3);
    outb(kMPicData, kIcw4);

    // set up slave
    outb(kSPicCmd, kIcw1);
    outb(kSPicData, kSIcw2);
    outb(kSPicData, kSIcw3);
    // NB Automatic EOI mode doesn't tend to work on the slave.
    // Linux source code says it's "to be investigated".
    outb(kSPicData, kIcw4);

    // use non-specific EOI by setting OCW2
    outb(kMPicCmd, 0x20u);
    outb(kSPicCmd, 0x20u);
    // OCW3:  0ef01prs
    //   ef:  0x = NOP, 10 = clear specific mask, 11 = set specific mask
    //    p:  0 = no polling, 1 = polling mode
    //   rs:  0x = NOP, 10 = read IRR, 11 = read ISR
    outb(kMPicCmd, 0x48u); // clear specific mask
    outb(kMPicCmd, 0x0au); // read IRR by default

    outb(kSPicCmd, 0x48u); // OCW3
    outb(kSPicCmd, 0x0au); // OCW3

    // mask all interrupts except IRQ2, so slave PIC will not be disabled
    outb(kMPicData, 0xfbu);
    outb(kSPicData, 0xffu);
}

void SendEoi(int intr_vec) {
    if (intr_vec >= kSIcw2)
        outb(kSPicCmd, kPicEoi);

    outb(kMPicCmd, kPicEoi);
}

void EnableIrq(int intr_vec) {
    uint8_t imr;
    if (intr_vec < kSIcw2) {
        imr = inb(kMPicData);
        imr &= ~(1u << (unsigned) (intr_vec - kMIcw2));
        outb(kMPicData, imr);
    } else {
        imr = inb(kSPicData);
        imr &= ~(1u << (unsigned) (intr_vec - kSIcw2));
        outb(kSPicData, imr);
    }
}

void DisableIrq(int intr_vec) {
    uint8_t imr;
    if (intr_vec < kSIcw2) {
        imr = inb(kMPicData);
        imr |= (1u << (unsigned) (intr_vec - kMIcw2));
        outb(kMPicData, imr);
    } else {
        imr = inb(kSPicData);
        imr |= (unsigned) (1u << (intr_vec - kSIcw2));
        outb(kSPicData, imr);
    }
}

void EnableAllIrqs() {
    outb(kMPicData, 0x0u);
    outb(kSPicData, 0x0u);
}