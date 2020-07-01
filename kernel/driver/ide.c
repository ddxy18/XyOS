//
// Created by dxy on 2020/4/8.
//

/**
 * We allocate a thread to handle IDE tasks. This thread will execute 
 * 'RunIde' since it's created and never return. It will sleep after
 * sending a request to the drive and be waken when drive sends back an 
 * interrupt. Now we use a Queue to store all requests and write or read a 
 * sector in its turn to executing. Requests will Queue in a sequence 
 * according to the time they come.
 */

#include "ide.h"

#include "../lib/asm_wrapper.h"
#include "../lib/queue.h"
#include "../interrupt/pic.h"
#include "../interrupt/interrupt.h"
#include "../thread/thread.h"

// register IO address
static const int kIoBase = 0x1f0;
static const int kCtrlBase = 0x3f6;
static const int kDataReg = kIoBase + 0;
static const int kErrReg = kIoBase + 1;
static const int kFeatureReg = kIoBase + 1;
static const int kSecCountReg = kIoBase + 2;
static const int kSecNoReg = kIoBase + 3;
static const int kCylinderLowReg = kIoBase + 4;
static const int kCylinderHighReg = kIoBase + 5;
static const int kDriveHeadSelReg = kIoBase + 6;
static const int kStatusReg = kIoBase + 7;
static const int kCmdReg = kIoBase + 7;
static const int kAlternateStatusReg = kCtrlBase + 0;
static const int kDeviceCtrlReg = kCtrlBase + 0;
static const int kDriveAddrReg = kCtrlBase + 1;

// status register bit
static const int kIdeBsy = 0x80;
static const int kIdeErr = 0x1;
static const int kIdeDf = 0x20;
static const int kIdeRdy = 0x40;
static const int kIdeDrq = 0x08;

// command
static const int kIdentity = 0xec;
static const int kRead = 0x20;
static const int kWrite = 0x30;

typedef struct IdeBlock {
    uint8_t dev_no;
    uint32_t sec_addr;
    uintptr_t mem_addr;
    // write if 'TRUE' and read if 'FALSE'
    bool op;
    unsigned int sec_num;
    ListNode node;
    Thread *req_thread;
} IdeBlock;

static Queue *block_queue;

/**
 * Start transfer between disk and memory.
 */
_Noreturn static void RunIde();

static int IdeWait(bool check_err);

static void IdeHandler(IntrReg *intr_reg);

static inline void DisableIntr() {
    outb(kDeviceCtrlReg, 2);
}

static inline void EnableIntr() {
    outb(kDeviceCtrlReg, 0);
}

bool IdeInit() {
    IdeWait(0);
    // disable ide from sending interrupts
    DisableIntr();

    // detect whether disk 1 is available
    outb(kDriveHeadSelReg, 0xf0);
    outb(kSecCountReg, 0);
    outb(kSecNoReg, 0);
    outb(kCylinderLowReg, 0);
    outb(kCylinderHighReg, 0);
    outb(kCmdReg, kIdentity);
    if (inb(kCylinderLowReg) != 0 || inb(kCylinderHighReg) != 0) {
        return FALSE;
    }
    if (inb(kStatusReg) == 0 || IdeWait(1) != 0) {
        return FALSE;
    }

    unsigned char buffer[512];
    insl(kDataReg, buffer, sizeof(buffer) / WORD_SIZE);

    block_queue = queue();

    // enable ide interrupt
    EnableIrq(kIdeVec);
    RegIntrHandler(kIdeVec, IdeHandler);
    EnableIntr();

    Thread *ide_thread = CreateThread(GetKPcb());
    KThreadRun(ide_thread, RunIde);

    return TRUE;
}

void RSec(uint8_t dev_no, uint32_t sec_addr) {
    IdeWait(0);
    outb(kDeviceCtrlReg, 0);
    outb(kSecCountReg, 1);
    outb(kSecNoReg, sec_addr);
    outb(kCylinderLowReg, sec_addr >> 8u);
    outb(kCylinderHighReg, sec_addr >> 16u);
    outb(kDriveHeadSelReg, 0xe0u | ((dev_no & 1u) << 4u) | (sec_addr >> 24u));
    outb(kCmdReg, kRead);
    ThreadSleep();
}

void WSec(uint8_t dev_no, uint32_t sec_addr, const void *src) {
    IdeWait(0);
    outb(kDeviceCtrlReg, 0);
    outb(kSecCountReg, 1);
    outb(kSecNoReg, sec_addr);
    outb(kCylinderLowReg, sec_addr >> 8u);
    outb(kCylinderHighReg, sec_addr >> 16u);
    outb(kDriveHeadSelReg,
         0xe0u | ((dev_no & 1u) << 4u) | (sec_addr >> 24u));
    outb(kCmdReg, kWrite);
    // write data
    while ((inb(kAlternateStatusReg) & (unsigned) kIdeRdy) == 0);
    if (IdeWait(1) == 0 &&
        (inb(kAlternateStatusReg) & (unsigned) kIdeDrq) != 0) {
        outsl(kDataReg, src, kSecSize / WORD_SIZE);
    }
    ThreadSleep();
}

static void IdeHandler(IntrReg *intr_reg) {
    SendEoi(intr_reg->intr_vec_no);

    if ((inb(kStatusReg) & ((unsigned) kIdeErr | (unsigned) kIdeDf)) != 0) {
        // error
        return;
    }

    IdeBlock *block = GET_STRUCT(QueueGetHead(block_queue), IdeBlock,
                                 node);
    if (block->op == FALSE) {
        // read from disk
        while ((inb(kAlternateStatusReg) & (unsigned) kIdeRdy) == 0);
        insl(kDataReg, (void *) block->mem_addr, kSecSize / WORD_SIZE);
    }

    if (--block->sec_num == 0) {
        ListNode *node = Dequeue(block_queue);
        RelBytes((uintptr_t) node);
        RelBytes((uintptr_t) block);
        ThreadWake(block->req_thread);
    }
    // TODO: wake up ide thread
}

static int IdeWait(bool check_err) {
    uint8_t status;
    while (((status = inb(kAlternateStatusReg)) & (unsigned) kIdeBsy) != 0) {
        if (check_err &&
            (status & ((unsigned) kIdeDf | (unsigned) kIdeErr)) != 0) {
            return -1;
        }
    }
    return 0;
}

_Noreturn static void RunIde() {
    ListNode *new_node;
    while (TRUE) {
        while ((new_node = QueueGetHead(block_queue)) != NULL) {
            IdeBlock *new_task = GET_STRUCT(new_node, IdeBlock, node);
            if (new_task->op) {
                WSec(new_task->dev_no, new_task->sec_addr,
                     (const void *) new_task->mem_addr);
            } else {
                RSec(new_task->dev_no, new_task->sec_addr);
            }
        }
    }
}

void IdeWrite(uint8_t dev_no, uint32_t sec_addr, const void *src,
              uint32_t block_num) {
    IdeBlock *block = (IdeBlock *) ReqBytes(sizeof(IdeBlock));
    block->op = TRUE;
    block->mem_addr = (uintptr_t) src;
    block->dev_no = dev_no;
    block->sec_addr = sec_addr;
    block->sec_num = block_num * kBlockSize;
    block->req_thread = GetCurThread();
    block->node.next = NULL;
    Enqueue(block_queue, &block->node);
}

void IdeRead(uint8_t dev_no, uint32_t sec_addr, void *dst, uint32_t
block_num) {
    IdeBlock *block = (IdeBlock *) ReqBytes(sizeof(IdeBlock));
    block->op = FALSE;
    block->mem_addr = (uintptr_t) dst;
    block->dev_no = dev_no;
    block->sec_addr = sec_addr;
    block->sec_num = block_num * kBlockSize;
    block->req_thread = GetCurThread();
    block->node.next = NULL;
    Enqueue(block_queue, &block->node);
}