//
// Created by dxy on 2020/4/2.
//

#ifndef XYOS_PIC_H
#define XYOS_PIC_H

#include <stdint.h>

// I/O Addresses of the two programmable interrupt controllers
#define M_PIC_CMD 0x20u
#define M_PIC_DATA 0x21u
#define S_PIC_CMD 0xa0u
#define S_PIC_DATA 0xa1u

#define IRQ_SLAVE 0x2u

#define M_IRQ_NUM 0x8u
#define S_IRQ_NUM 0x8u
#define IRQ_NUM (M_IRQ_NUM + S_IRQ_NUM)
/**
 * @def ICW1
 * @verbatim
 * format: 0001c0ba
 * c  0 = edge triggering, 1 = level triggering
 * b  0 = cascaded PICs, 1 = master only
 * a  0 = no ICW4, 1 = ICW4 required
 * @endverbatim
 */
#define ICW1 0x11u
/**
 * @def ICW2: vector offset
 */
#define M_ICW2 0x20u
#define S_ICW2 (M_ICW2 + 8)
/**
 * @def ICW3
 * @verbatim
 * (master PIC) bit mask of IR lines connected to slaves
 * (slave PIC) 3-bit # of slave's connection to master
 * @endverbatim
 */
#define M_ICW3 (1u << IRQ_SLAVE)
#define S_ICW3 IRQ_SLAVE
/**
 * @def ICW4
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
#define ICW4 0x1u

#define PIC_EOI 0x20u
#define PIC_READ_IRR 0xau
#define PIC_READ_TSR 0xbu

void pic_init();

void send_eoi(uint16_t intr_vec);

void enable_irq(uint16_t intr_vec);

void disable_irq(uint16_t intr_vec);

void enable_all_irqs();

#endif //XYOS_PIC_H
