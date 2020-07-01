//
// Created by dxy on 2020/4/3.
//

#ifndef XYOS_ASM_WRAPPER_H
#define XYOS_ASM_WRAPPER_H

#include <stdint.h>

uint8_t inb(uint16_t port);

void outb(uint16_t port, uint8_t data);

/**
 * Get given control register value.
 *
 * @param cr_no It can be 0 1 2 3 4 and separately represents '%cr0' '%cr1' '%cr2' '%cr3' '%cr4'.
 * @return uint32_t given control register's value
 */
uint32_t get_cr(int cr_no);

/**
 * Set appointed bit in cr[x].
 *
 * @param cr_no
 * @param i range from 0 to 31
 */
void set_cr(int cr_no, int i);

/**
 * Clear appointed bit in cr[x].
 *
 * @param cr_no
 * @param i
 */
void clear_cr(int cr_no, int i);

/**
 * @return uint32_t EFLAGS value
 */
uint32_t get_eflags();

/**
 * Get segment register's value.
 *
 * @param c It can be 'c' 'd' 'e' 'f' 'g' 's' and separately represents '%cs' '%ds' '%es' '%fs' '%gs' '%ss'.
 * @return uint16_t given segment register's value
 */
uint16_t get_seg_reg(char c);

/**
 * Nothing except a return will be executed in it. So it is used to refresh segment register and '%eip'.
 * Notice that it must be called at the end of a void return type function.
 *
 * @param cs value of '%cs'
 * @param offset difference between the current offset and the target offset
 */
void ljmp(uint16_t cs, uint32_t offset);

uint32_t get_esp();

static inline void insl(uint32_t port, void *addr, int word_num) {
    asm volatile (
    "cld;"
    "repne; insl;"
    : "=D" (addr), "=c" (word_num)
    : "d" (port), "0" (addr), "1" (word_num)
    : "memory", "cc");
}

static inline void outsl(uint32_t port, const void *addr, int word_num) {
    asm volatile (
    "cld;"
    "repne; outsl;"
    : "=S" (addr), "=c" (word_num)
    : "d" (port), "0" (addr), "1" (word_num)
    : "memory", "cc");
}

static inline void intr_enable() {
    asm("sti");
}

static inline void intr_disable() {
    asm("cli");
}

#endif //XYOS_ASM_WRAPPER_H