//
// Created by dxy on 2020/4/3.
//

#include "asm_wrapper.h"

uint8_t inb(uint16_t port) {
    uint8_t data;
    asm volatile("inb %1, %0" : "=a"(data) : "d"(port) : "memory");
    return data;
}

void outb(uint16_t port, uint8_t data) {
    asm volatile("outb %0, %1"::"a"(data), "d"(port) : "memory");
}

uint32_t get_cr(int cr_no) {
    uint32_t cr_val = 0;
    switch (cr_no) {
        case 0:
            asm("mov %cr0, %eax");
            asm("movl %%eax, %0":"=r"(cr_val)::"memory");
            break;
        case 1:
            asm("movl %cr1, %eax");
            asm("movl %%eax, %0":"=r"(cr_val)::"memory");
            break;
        case 2:
            asm("movl %cr2, %eax");
            asm("movl %%eax, %0":"=r"(cr_val)::"memory");
            break;
        case 3:
            asm("movl %cr3, %eax");
            asm("movl %%eax, %0":"=r"(cr_val)::"memory");
            break;
        case 4:
            asm("movl %cr4, %eax");
            asm("movl %%eax, %0":"=r"(cr_val)::"memory");
            break;
        default:
            break;
    }
    return cr_val;
}

void set_cr(int cr_no, int i) {
    uint32_t cr = (1u << (unsigned) i);
    switch (cr_no) {
        case 0:
            asm("movl %cr0, %eax");
            asm("orl %0, %%eax"::"m"(cr));
            asm("movl %eax, %cr0");
            break;
        case 1:
            asm("movl %cr1, %eax");
            asm("orl %0, %%eax"::"m"(cr));
            asm("movl %eax, %cr1");
            break;
        case 2:
            asm("movl %cr2, %eax");
            asm("orl %0, %%eax"::"m"(cr));
            asm("movl %eax, %cr2");
            break;
        case 3:
            asm("movl %cr3, %eax");
            asm("orl %0, %%eax"::"m"(cr));
            asm("movl %eax, %cr3");
            break;
        case 4:
            asm("movl %cr4, %eax");
            asm("orl %0, %%eax"::"m"(cr));
            asm("movl %eax, %cr4");
            break;
        default:
            break;
    }
}

void clear_cr(int cr_no, int i) {
    uint32_t cr = ~(1u << (unsigned) i);
    switch (cr_no) {
        case 0:
            asm("movl %cr0, %eax");
            asm("andl %0, %%eax"::"m"(cr));
            asm("movl %eax, %cr0");
            break;
        case 1:
            asm("movl %cr1, %eax");
            asm("andl %0, %%eax"::"m"(cr));
            asm("movl %eax, %cr1");
            break;
        case 2:
            asm("movl %cr2, %eax");
            asm("andl %0, %%eax"::"m"(cr));
            asm("movl %eax, %cr2");
            break;
        case 3:
            asm("movl %cr3, %eax");
            asm("andl %0, %%eax"::"m"(cr));
            asm("movl %eax, %cr3");
            break;
        case 4:
            asm("movl %cr4, %eax");
            asm("andl %0, %%eax"::"m"(cr));
            asm("movl %eax, %cr4");
            break;
        default:
            break;
    }
}

uint32_t get_eflags() {
    uint32_t eflags;
    asm("pushfl");
    asm("movl (%%esp), %0":"=r"(eflags)::"memory");
    asm("addl $4, %esp");
    return eflags;
}

uint16_t get_seg_reg(char c) {
    uint16_t seg_reg = 0;
    switch (c) {
        case 'c':
            asm("movw %%cs, %0":"=r"(seg_reg)::"memory");
            break;
        case 'd':
            asm("movw %%ds, %0":"=r"(seg_reg)::"memory");
            break;
        case 'e':
            asm("movw %%es, %0":"=r"(seg_reg)::"memory");
            break;
        case 'f':
            asm("movw %%fs, %0":"=r"(seg_reg)::"memory");
            break;
        case 'g':
            asm("movw %%gs, %0":"=r"(seg_reg)::"memory");
            break;
        case 's':
            asm("movw %%ss, %0":"=r"(seg_reg)::"memory");
            break;
        default:
            break;
    }
    return seg_reg;
}

uint32_t get_esp() {
    uint32_t esp;
    asm("movl %%esp, %0":"=r"(esp));
    return esp;
}