//
// Created by dxy on 2020/4/10.
//

#ifndef XYOS_THREAD_H
#define XYOS_THREAD_H

#include "../process/process.h"
#include "../lib/list.h"

typedef struct KThread {
    unsigned int tid;
    uintptr_t stack_addr;
    // If the thread belongs to kernel process, 'Pcb' will be 'NULL'.
    Pcb *pcb;
    uintptr_t esp;
    ListNode node;
} KThread;

void ThreadInit();

KThread *ReqThread(Pcb *pcb);

void RelThread(KThread *thread);

void ThreadSwitch();

/**
 * It is only used to initialize kernel thread's run environment. Once these
 * threads start to run, they will be never released unless an OS crash happens.
 *
 * @param thread
 * @param start_func the first function the thread will execute
 */
void KThreadRun(KThread *thread, void (*start_func)());

void ThreadSleep();

void ThreadWake(KThread *thread);

void ThreadWakeAll(KThread **thread_list, int num);

static inline KThread *GetCurThread() {
    /*
     * We will store current stack's upper address into Tss after a thread
     * switch, so we can find current thread structure through TSS.
     */
    uintptr_t *t = (uintptr_t *) tss->esp0;
    return (KThread *) *t;
}

#endif //XYOS_THREAD_H