#define XYOS_INTR_ENTRY_H
#include "../memory/segment.h"
.text
.globl _alltraps
_alltraps:
    # push general purpose registers
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
    movl $kKDataSeg, %eax
    movw %ax, %ds
    movw %ax, %es
    movw %ax, %fs
    movw %ax, %gs

    # push %esp to pass a pointer to the IntrReg as an argument to IntrHandlerSel()
    pushl %esp
    # call IntrHandlerSel()
    call _IntrHandlerSel

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
