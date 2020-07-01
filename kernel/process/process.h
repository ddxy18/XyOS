//
// Created by dxy on 2020/3/22.
//

#ifndef XYOS_PROCESS_H
#define XYOS_PROCESS_H

#include "../lib/def.h"
#include "../memory/paging.h"
#include "../interrupt/idt.h"
#include "../lib/list.h"

/**
 * 4 KB kernel stack, mostly used to handle interrupts and syscall
 */
static const int kKStackSize = 0x1000;

/**
 * Now we use the same virtual address for all processes' user stack.
 */
static const uintptr_t kUStackAddr = 0xbffffff0;
static const int kUStackInitSize = 0x1000;
static const int kIoInitSize = 0x10000;

typedef struct MemBlock {
    uintptr_t addr;
    unsigned int size;
} MemBlock;

enum ProcState {
    WAIT, RUN, DIE
} ProcState;

/**
 * @typedef Pcb
 *
 * process control block
 */
typedef struct Pcb {
    unsigned int pid;
    Pde *page_dir;
    enum ProcState state;
    /**
     * Runtime constants. So after creating process, it never changes unless
     * calling 'exec' to load new executable file.
     */
    MemBlock code;
    MemBlock data;
    struct Pcb *father;
    // structure process tree
    ListNode left;
    ListNode right;
    // used for state list
    ListNode node;
    // store all threads
    ListNode thread_list;
} Pcb;

void ProcManagerInit();

Pcb *CreateProc(Pcb *father);

/**
 * @brief For the process to release itself.
 *
 * Notice that its pcb and its current stack is reserved for its father to
 * release.
 */
void CloseProc();

/**
 * To make kernel threads same to user threads, we create a 'Pcb' structure for
 * the kernel, all kernel threads use it as 'pcb' member in their 'Thread'
 * structure.
 *
 * @return Pcb * 'k_pcb'
 */
Pcb *GetKPcb();

#endif //XYOS_PROCESS_H
