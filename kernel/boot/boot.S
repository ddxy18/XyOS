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
#define INIT_STACK_SIZE 0x4000
multiboot_header:
    /**
     * magic
     * must be the hexadecimal value 0x1badb002
     */
    .long MULTIBOOT_HEADER_MAGIC
    /**
     * architecture
     * specify the Central Processing Unit Instruction Set Architecture
     * ISA: i386
     */
    .long MULTIBOOT_ARCHITECTURE_I386
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
    .long -(MULTIBOOT_HEADER_MAGIC + MULTIBOOT_ARCHITECTURE_I386)
multiboot_header_end:

_start:
    /* initialize the stack pointer */
    movl init_stack, %esp
    addl INIT_STACK_SIZE, %esp
    call _kernel_start
    
.data
    init_stack:
        .long 0x7c00