# The Multiboot header must be contained completely within the first 8192 bytes
# of the OS image and must be longword (32-bit) aligned.
# The Multiboot2 header must be contained completely within the first 32768
# bytes of the OS image and must be 64-bit aligned.
 
#define XYOS_BOOT_H
#include "multiboot.h"
    .text
    .align 4
    .extern _kernel_start
    .globl _start
    .section .multiboot
    .align 4
multiboot_header:
    
    # magic must be the hexadecimal value 0x1badb002
    .long MULTIBOOT_HEADER_MAGIC
    # flags
    .long MULTIBOOT_HEADER_FLAGS
    # checksum
    # When it is added to the other magic fields (i.e. ‘magic’ and 'flags'), they must have a 32-bit unsigned sum of zero.
    .long -(MULTIBOOT_HEADER_MAGIC + MULTIBOOT_HEADER_FLAGS)
multiboot_header_end:

_start:
    # initialize the stack pointer
    movl init_stack, %esp
    # set stack size to 16 KB, reserve stack botttom for 'Thread *'
    addl $0x3ffc, %esp
    # reset EFLAGS
    pushl $0
    popf
    # set address of the Multiboot information structure as arg2
    pushl %ebx
    # set magic as arg1
    pushl %eax
    call _kernel_start
    cmpl $0, %eax
    # machine state violates Multiboot starndard
    jne loop
loop:
    hlt
    jmp loop

    .data
        # kernel central stack for c environment
        init_stack:
            .long 0x7c00