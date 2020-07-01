//
// Created by dxy on 2020/4/10.
//

#ifndef XYOS_THREAD_H
#define XYOS_THREAD_H

#include "../process/process.h"
#include "../lib/list.h"

typedef struct Thread {
    unsigned int tid;
    uintptr_t stack_addr;
    // If the thread belongs to kernel process, 'Pcb' will be 'NULL'.
    Pcb *pcb;
    uintptr_t esp;
    ListNode node;
    // Link thread in a process as a circular list.
    ListNode sibling;
} Thread;

void ThreadInit();

Thread *CreateThread(Pcb *pcb);

void DestructThread(Thread *thread);

void ThreadSwitch();

/**
 * It is only used to initialize kernel thread's run environment. Once these
 * threads start to run, they will be never released unless an OS crash happens.
 *
 * @param thread
 * @param start_func the first function the thread will execute
 */
void KThreadRun(Thread *thread, void (*start_func)());

void ThreadSleep();

void ThreadWake(Thread *thread);

void ThreadWakeAll(Thread **thread_list, int num);

static inline Thread *GetCurThread() {
    /*
     * We will store current stack's upper address into Tss after a thread
     * switch, so we can find current thread structure through TSS.
     */
    uintptr_t *t = (uintptr_t *) tss->esp0;
    return (Thread *) *t;
}

#endif //XYOS_THREAD_H