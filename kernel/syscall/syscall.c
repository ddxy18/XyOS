//
// Created by dxy on 2020/4/15.
//

#include "syscall.h"

#include <stddef.h>

#include "../interrupt/interrupt.h"

static const int kSyscallMaxNum = 128;

static int (*syscall[])(int num, ...);

/**
 * 'eax' -- syscall vector number
 * 'ebx' -- args numbers
 * 'ecx' 'edx' 'edi' 'esi' -- args
 *
 * @param intr_reg
 */
static void Syscall(IntrReg *intr_reg);

void SyscallInit() {
    RegIntrHandler(kSyscallVec, Syscall);
    // TODO: init 'syscall'
}

static void Syscall(IntrReg *intr_reg) {
    // invalid syscall vector number
    if (intr_reg->p_eax >= kSyscallMaxNum) {
        intr_reg->p_eax = -1;
        return;
    }

    if (syscall[intr_reg->p_eax] != NULL) {
        switch (intr_reg->p_ebx) {
            case 0:
                intr_reg->p_eax = syscall[intr_reg->p_eax](0);
            case 1:
                intr_reg->p_eax = syscall[intr_reg->p_eax](1, intr_reg->p_ecx);
            case 2:
                intr_reg->p_eax = syscall[intr_reg->p_eax](2, intr_reg->p_ecx,
                                                           intr_reg->p_edx);
            case 3:
                intr_reg->p_eax = syscall[intr_reg->p_eax](3, intr_reg->p_ecx,
                                                           intr_reg->p_edx,
                                                           intr_reg->p_edi);
            case 4:
                intr_reg->p_eax = syscall[intr_reg->p_eax](4, intr_reg->p_ecx,
                                                           intr_reg->p_edx,
                                                           intr_reg->p_edi,
                                                           intr_reg->p_esi);
        }
    }
}
