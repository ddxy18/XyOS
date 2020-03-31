/**
 * The Multiboot header must be contained completely within the first 8192 bytes of the OS image,
 * and must be longword (32-bit) aligned.
 * The Multiboot2 header must be contained completely within the first 32768 bytes of the OS image,
 * and must be 64-bit aligned.
 */
 
#include "multiboot.h"
    .text
    .align 4
    .extern _kernel_start
    .globl _start
#define MULTIBOOT_HEADER_FLAGS MULTIBOOT_PAGE_ALIGN | MULTIBOOT_MEMORY_INFO
    .section .multiboot
    .align 4
multiboot_header:
    /**
     * magic
     * must be the hexadecimal value 0x1badb002
     */
    .long MULTIBOOT_HEADER_MAGIC
    /* flags */
    .long MULTIBOOT_HEADER_FLAGS
    /**
     * architecture
     * specify the Central Processing Unit Instruction Set Architecture
     * ISA: i386
     */
    #.long MULTIBOOT_ARCHITECTURE_I386
    /**
     * supported by multiboot2
     * header_length
     * specify the length of Multiboot2 header in bytes including magic fields
     .long multiboot_header_end - multiboot_header
     */
    /**
     * checksum
     * when added to the other magic fields (i.e. ‘magic’, ‘architecture’ and ‘header_length’),
     * must have a 32-bit unsigned sum of zero
     */
    /* -(MULTIBOOT2_HEADER_MAGIC + MULTIBOOT_ARCHITECTURE_I386 + multiboot_header_end - multiboot_header) */
    .long -(MULTIBOOT_HEADER_MAGIC + MULTIBOOT_HEADER_FLAGS)
multiboot_header_end:

_start:
    /* initialize the stack pointer */
    movl init_stack, %esp
    # set stack size to 16 KB
    addl $0x4000, %esp
    /* reset EFLAGS */
    pushl $0
    popf
    /* set address of the Multiboot information structure as arg2 */
    pushl %ebx
    /* set magic as arg1 */
    pushl %eax
    call _kernel_start
    cmpl $0, %eax
    /* machine state violates Multiboot starndard */
    jne loop
loop:
    hlt
    jmp loop

    .data
        /* temporary kernel stack for c environment */
        init_stack:
            .long 0x7c00