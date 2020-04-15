//
// Created by dxy on 2020/4/10.
//

#include "thread.h"

#include "../lib/asm_wrapper.h"
#include "../lib/queue.h"

static const int kThreadMaxNum = 65536;
static const int kThreadPoolInitNum = 128;

static const uintptr_t kMainStackAddr = 0xbc00;

/**
 * It works like a thread pool. When a thread request comes, it can allocate
 * 'KThread' structure quickly.
 */
static ListNode *free_thread_list;

static Queue *wait_thread_queue;

static ListNode *block_thread_list;

extern void SwitchReg(uintptr_t *cur_esp, uintptr_t *new_esp);

void ThreadInit() {
    // allocate memory for thread pool
    KThread *thread = (KThread *) ReqBytes(
            kThreadPoolInitNum * (int) sizeof(KThread));
    if (thread == NULL) {
        free_thread_list = NULL;
        return;
    }

    // link all free thread structure to a List
    free_thread_list = List();
    ListNode *current = &thread->node;
    uintptr_t addr = (uintptr_t) current;
    int i = 0;
    while (i < kThreadPoolInitNum) {
        GET_STRUCT(current, KThread, node)->tid = kThreadPoolInitNum - i;
        ListAdd(free_thread_list, current);
        addr += sizeof(KThread);
        current = (ListNode *) addr;
        i++;
    }

    wait_thread_queue = queue();
    block_thread_list = List();

    // set current thread's structure
    KThread *k_main = (KThread *) ReqBytes(sizeof(KThread));
    k_main->pcb = GetKPcb();
    k_main->stack_addr = kMainStackAddr;
    k_main->esp = kMainStackAddr - sizeof(KThread *);
    uintptr_t *t = (uintptr_t *) k_main->esp;
    *t = (uintptr_t) k_main;
    k_main->tid = 0;
    k_main->node.next = NULL;
    tss->esp0 = k_main->esp;
}

KThread *ReqThread(Pcb *pcb) {
    if (IsListEmpty(free_thread_list) == FALSE) {
        ListNode *node = ListRemove(free_thread_list);
        KThread *new_thread = GET_STRUCT(node, KThread, node);
        new_thread->stack_addr = kKStackSize + ReqBytes(kKStackSize) - 1;
        new_thread->pcb = pcb;
        // Store 'new_thread''s address in the top of its stack.
        new_thread->esp = new_thread->stack_addr - sizeof(KThread *);
        uintptr_t *t = (uintptr_t *) new_thread->esp;
        *t = (uintptr_t) new_thread;
        return new_thread;
    }
    return NULL;
}

void RelThread(KThread *thread) {
    thread->pcb = NULL;
    RelBytes(thread->stack_addr - kKStackSize + 1);
    ListAdd(free_thread_list, &thread->node);
}

void ThreadSwitch() {
    ListNode *new_node = Dequeue(wait_thread_queue);
    if (new_node == NULL) {
        // No thread waiting to be executed.
        return;
    }
    KThread *new_thread = GET_STRUCT(new_node, KThread, node);
    KThread *cur_thread = GetCurThread();

    Enqueue(wait_thread_queue, &cur_thread->node);

    // If process switch happens, restore page directory.
    if (cur_thread->pcb->pid != new_thread->pcb->pid) {
        SwitchPd(DirectMappingPhysAddr((uintptr_t) new_thread->pcb->page_dir));
    }

    /* If the thread returns to user space in this time slice, its stack must
     * be empty, so we can store 'stack_addr' in TSS. Otherwise, the TSS
     * isn't used, so changing its value has no influence.
     */
    tss->esp0 = (uint32_t) (new_thread->stack_addr - sizeof(KThread *));

    // complete general registers switch
    SwitchReg(&cur_thread->esp, &new_thread->esp);

    /**
     * Thread switch may happen due to a timer interrupt. We have to enable
     * external interrupts manually since new thread may not stop due to a
     * timer interrupt.
     */
    intr_enable();
}

void KThreadRun(KThread *thread, void (*start_func)()) {
    if (thread->pcb->pid == 0) {
        // Save return address of 'SwitchReg' to 'start_func'.
        thread->esp -= sizeof(uintptr_t);
        uintptr_t *t = (uintptr_t *) thread->esp;
        *t = (uintptr_t) start_func;
        //
        thread->esp -= 4 * sizeof(uint32_t);
        t = (uintptr_t *) thread->esp;
        *t = 0;
        *(t + 1) = 0;
        *(t + 2) = 0;
        *(t + 3) = 0;

        Enqueue(wait_thread_queue, &thread->node);
    }
}

void ThreadSleep() {
    KThread *cur_thread = GetCurThread();
    // Switch current thread to block List.
    ListAdd(block_thread_list, &cur_thread->node);

    ThreadSwitch();
}

void ThreadWake(KThread *thread) {
    ListNode *tmp = block_thread_list;
    while (tmp->next != block_thread_list && tmp->next != &thread->node) {
        tmp = tmp->next;
    }
    if (tmp->next == &thread->node) {
        ListRemove(tmp);
        Enqueue(wait_thread_queue, &thread->node);
    }
}

void ThreadWakeAll(KThread **thread_list, int num) {
    for (int i = 0; i < num; ++i) {
        ThreadWake(thread_list[i]);
    }
}