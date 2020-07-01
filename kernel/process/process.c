//
// Created by dxy on 2020/3/22.
//

#include "process.h"

#include <stddef.h>

#include "../lib/asm_wrapper.h"
#include "../thread/thread.h"

static const int kProcMaxNum = 0x1000;
static const int kProcInitNum = 0x10;

static Pcb *k_pcb;

static ListNode *free_proc_list;

static void AddSon(ListNode *son, ListNode *father);

/**
 * It is blocked when some of the sons are still 'RUN'.
 *
 * @param pcb
 */
static void DestructAllSons(Pcb *pcb);

void ProcManagerInit() {
    Pcb *pcb_list = (Pcb *) ReqBytes(kProcInitNum * (int) sizeof(Pcb));
    free_proc_list = List();

    ListNode *cur = &pcb_list->node;
    for (int i = 0; i < kProcInitNum; ++i) {
        ListAdd(free_proc_list, cur);
        Pcb *tmp = GET_STRUCT(cur, Pcb, node);
        tmp->pid = kProcInitNum - i;
        tmp->thread_list.next = NULL;
        tmp->left.next = NULL;
        tmp->right.next = NULL;
        tmp->father = NULL;
        tmp->page_dir = NULL;
        cur++;
    }

    // set kernel process's structure
    k_pcb = (Pcb *) ReqBytes(sizeof(Pcb));
    k_pcb->pid = 0;
    k_pcb->node.next = NULL;
    k_pcb->page_dir = (Pde *) get_cr(3);
    k_pcb->thread_list.next = &k_pcb->thread_list;
    // 'k_pcb' is independent of the process tree.
    k_pcb->father = NULL;
    k_pcb->left.next = NULL;
    k_pcb->right.next = NULL;
    k_pcb->state = RUN;
}

Pcb *CreateProc(Pcb *father) {
    if (IsListEmpty(free_proc_list) == TRUE) {
        return NULL;
    }

    Pcb *new_pcb = GET_STRUCT(ListRemove(free_proc_list), Pcb, node);
    new_pcb->state = WAIT;
    new_pcb->thread_list.next = &new_pcb->thread_list;
    // clone page directory
    new_pcb->page_dir = (Pde *) ReqBytes(kPdSize);
    UPdClone(new_pcb->page_dir, father->page_dir);
    // add it to process tree
    new_pcb->father = father;
    AddSon(&new_pcb->right, &father->left);
    new_pcb->node.next = NULL;
    // share user space between 'new_pcb' and 'father'
    new_pcb->code.addr = father->code.addr;
    new_pcb->code.size = father->code.size;
    new_pcb->data.addr = father->data.addr;
    new_pcb->data.size = father->data.size;
    return new_pcb;
}

void CloseProc() {
    Thread *cur_thread = GetCurThread();
    Pcb *pcb = cur_thread->pcb;
    DestructAllSons(pcb);
    /**
     * Use kernel's page directory so that we can release process's page
     * directory safely.
     */
    SwitchPd(kKPdAddr);
    // destruct all threads exclude the current one
    ListNode *tmp = &pcb->thread_list;
    while (tmp->next != &pcb->thread_list) {
        if (tmp->next != &cur_thread->sibling) {
            DestructThread(GET_STRUCT(tmp->next, Thread, sibling));
        } else {
            tmp = tmp->next;
        }
    }
    // release physical pages and corresponding page tables and page directory
    Pde *page_dir = pcb->page_dir;
    for (int i = 0; i < kPdeNum; ++i) {
        if (page_dir[i].pt_addr != kPdeAbsentAddr) {
            RelPages((Pte *) page_dir[i].pt_addr);
            RelBytes(DirectMappingVirtualAddr(page_dir[i].pt_addr));
        }
    }
    RelBytes((uintptr_t) page_dir);

    pcb->state = DIE;
}

Pcb *GetKPcb() {
    return k_pcb;
}

static void AddSon(ListNode *son, ListNode *father) {
    ListNode *tmp = father;
    while (tmp->next != NULL) {
        tmp = tmp->next;
    }
    tmp->next = son;
}

/**
 * It releases all sons of 'pcb', say, release their last thread and 'Pcb'
 * structure. It blocks if any one of them isn't 'DIE'.
 */
static void DestructAllSons(Pcb *pcb) {
    // no son exists
    if (pcb->left.next == NULL) {
        return;
    }

    ListNode *tmp = pcb->left.next;
    while (tmp != NULL) {
        Pcb *son = GET_STRUCT(tmp, Pcb, right);
        // wait for it to die
        while (son->state != DIE);
        DestructThread(GET_STRUCT(son->thread_list.next, Thread, sibling));
        son->thread_list.next = NULL;
        tmp = tmp->next;
        son->right.next = NULL;
        // release its structure
        ListAdd(free_proc_list, &son->node);
    }
    pcb->left.next = NULL;
}