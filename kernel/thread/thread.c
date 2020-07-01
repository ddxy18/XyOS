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
 * 'Thread' structure quickly.
 */
static ListNode *free_thread_list;

static Queue *wait_thread_queue;

static ListNode *block_thread_list;

extern void SwitchReg(uintptr_t *cur_esp, uintptr_t *new_esp);

void ThreadInit() {
    // allocate memory for thread pool
    Thread *thread = (Thread *) ReqBytes(
            kThreadPoolInitNum * (int) sizeof(Thread));
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
        GET_STRUCT(current, Thread, node)->tid = kThreadPoolInitNum - i;
        ListAdd(free_thread_list, current);
        addr += sizeof(Thread);
        current = (ListNode *) addr;
        i++;
    }

    wait_thread_queue = queue();
    block_thread_list = List();

    // set current thread's structure
    Thread *k_main = (Thread *) ReqBytes(sizeof(Thread));
    k_main->pcb = GetKPcb();
    k_main->stack_addr = kMainStackAddr;
    k_main->esp = kMainStackAddr - sizeof(Thread *);
    uintptr_t *t = (uintptr_t *) k_main->esp;
    *t = (uintptr_t) k_main;
    k_main->tid = 0;
    k_main->node.next = NULL;
    tss->esp0 = k_main->esp;
}

Thread *CreateThread(Pcb *pcb) {
    if (IsListEmpty(free_thread_list) == FALSE) {
        ListNode *node = ListRemove(free_thread_list);
        Thread *new_thread = GET_STRUCT(node, Thread, node);
        new_thread->stack_addr = kKStackSize + ReqBytes(kKStackSize) - 1;
        new_thread->pcb = pcb;
        // store 'new_thread''s address in the top of its stack
        new_thread->esp = new_thread->stack_addr - sizeof(Thread *);
        uintptr_t *t = (uintptr_t *) new_thread->esp;
        *t = (uintptr_t) new_thread;
        // link it to the process's 'thread_list'
        ListAdd(&pcb->thread_list, &new_thread->node);
        return new_thread;
    }
    return NULL;
}

void DestructThread(Thread *thread) {
    thread->pcb = NULL;
    RelBytes(thread->stack_addr - kKStackSize + 1);
    // remove 'thread' from its process's 'thread_list'
    ListNode *prev = &thread->pcb->thread_list;
    while (prev->next != &thread->sibling) {
        prev = prev->next;
    }
    ListRemove(prev);
    ListAdd(free_thread_list, &thread->node);
}

void ThreadSwitch() {
    ListNode *new_node = Dequeue(wait_thread_queue);
    if (new_node == NULL) {
        // No thread waiting to be executed.
        return;
    }
    Thread *new_thread = GET_STRUCT(new_node, Thread, node);
    Thread *cur_thread = GetCurThread();

    Enqueue(wait_thread_queue, &cur_thread->node);

    // If process switch happens, restore page directory.
    if (cur_thread->pcb->pid != new_thread->pcb->pid) {
        SwitchPd(DirectMappingPhysAddr((uintptr_t) new_thread->pcb->page_dir));
        new_thread->pcb->state = RUN;
        cur_thread->pcb->state = WAIT;
    }

    /* If the thread returns to user space in this time slice, its stack must
     * be empty, so we can store 'stack_addr' in TSS. Otherwise, the TSS
     * isn't used, so changing its value has no influence.
     */
    tss->esp0 = (uint32_t) (new_thread->stack_addr - sizeof(Thread *));

    // complete general registers switch
    SwitchReg(&cur_thread->esp, &new_thread->esp);

    /**
     * Thread switch may happen due to a timer interrupt. We have to enable
     * external interrupts manually since new thread may not stop due to a
     * timer interrupt.
     */
    intr_enable();
}

void KThreadRun(Thread *thread, void (*start_func)()) {
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
    Thread *cur_thread = GetCurThread();
    // Switch current thread to block List.
    ListAdd(block_thread_list, &cur_thread->node);

    ThreadSwitch();
}

void ThreadWake(Thread *thread) {
    ListNode *tmp = block_thread_list;
    // TODO: search through the list for the thread may consume large amount
    //  of time
    while (tmp->next != block_thread_list && tmp->next != &thread->node) {
        tmp = tmp->next;
    }
    if (tmp->next == &thread->node) {
        ListRemove(tmp);
        Enqueue(wait_thread_queue, &thread->node);
    }
}

void ThreadWakeAll(Thread **thread_list, int num) {
    for (int i = 0; i < num; ++i) {
        ThreadWake(thread_list[i]);
    }
}