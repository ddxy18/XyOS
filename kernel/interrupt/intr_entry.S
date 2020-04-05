#define XYOS_INTR_ENTRY_H
#include "../memory/segment.h"
.text
.globl _alltraps
_alltraps:
    # push eflags
    pushal
    # push segment registers
    movw %gs, %ax
    pushw %ax
    movw %fs, %ax
    pushw %ax
    movw %es, %ax
    pushw %ax
    movw %ds, %ax
    pushw %ax

    # change segment register to kernel segment
    movl $K_DATA_SEG, %eax
    movw %ax, %ds
    movw %ax, %es
    movw %ax, %fs
    movw %ax, %gs

    # push %esp to pass a pointer to the intr_reg_t as an argument to intr_handler_sel()
    pushl %esp
    # call intr_handler_sel()
    call _intr_handler_sel

    # pop argument
    pop %esp
    # restore segment register
    popw %ax
    movw %ax, %ds
    popw %ax
    movw %ax, %es
    popw %ax
    movw %ax, %fs
    popw %ax
    movw %ax, %gs

    # restore registers from stack
    popal

    # pop the trap number and error code
    addl $0x8, %esp
    iret