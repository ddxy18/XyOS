//
// Created by dxy on 2020/4/8.
//

#include "ide.h"
#include "../lib/asm_wrapper.h"
#include "../lib/queue.h"
#include "../interrupt/pic.h"
#include "../memory/direct_mapping_allocator.h"
#include "../interrupt/interrupt.h"

// register IO address
#define IO_BASE 0x1f0
#define CTRL_BASE 0x3f6
#define DATA_REG (IO_BASE + 0)
#define ERR_REG (IO_BASE + 1)
#define FEATURE_REG (IO_BASE + 1)
#define SEC_COUNT_REG (IO_BASE + 2)
#define SEC_NO_REG (IO_BASE + 3)
#define CYLINDER_LOW_REG (IO_BASE + 4)
#define CYLINDER_HIGH_REG (IO_BASE + 5)
#define DRIVE_HEAD_SEL_REG (IO_BASE + 6)
#define STATUS_REG (IO_BASE + 7)
#define CMD_REG (IO_BASE + 7)
#define ALTERNATE_STATUS_REG (CTRL_BASE + 0)
#define DEVICE_CTRL_REG (CTRL_BASE + 0)
#define DRIVE_ADDR_REG (CTRL_BASE + 1)

// status register bit
#define IDE_BSY 0x80u
#define IDE_ERR 0x1u
#define IDE_DF 0x20u
#define IDE_RDY 0x40u
#define IDE_DRQ 0x08u

// command
#define IDENTITY 0xec
#define READ 0x20
#define WRITE 0x30

typedef struct ide_sec {
    uint8_t dev_no;
    uint32_t sec_addr;
    uintptr_t mem_addr;
    // write if 'TRUE' and read if 'FALSE'
    bool op;
} ide_sec_t;

static int ide_wait(bool);

static void ide_handler(intr_reg_t *);

static k_queue_t *sec_queue;

static inline void disable_intr() {
    outb(DEVICE_CTRL_REG, 2);
}

static inline void enable_intr() {
    outb(DEVICE_CTRL_REG, 0);
}


bool ide_init() {
    ide_wait(0);
    // disable interrupts
    disable_intr();

    // detect whether disk 1 is available
    outb(DRIVE_HEAD_SEL_REG, 0xf0);
    outb(SEC_COUNT_REG, 0);
    outb(SEC_NO_REG, 0);
    outb(CYLINDER_LOW_REG, 0);
    outb(CYLINDER_HIGH_REG, 0);
    outb(CMD_REG, IDENTITY);
    if (inb(CYLINDER_LOW_REG) != 0 || inb(CYLINDER_HIGH_REG) != 0) {
        return FALSE;
    }
    if (inb(STATUS_REG) == 0 || ide_wait(1) != 0) {
        return FALSE;
    }

    unsigned char buffer[512];
    insl(DATA_REG, buffer, sizeof(buffer) / sizeof(unsigned char));

    sec_queue = queue();

    // enable ide interrupt
    enable_irq(IDE_VEC);
    reg_intr_handler(IDE_VEC, ide_handler);
    enable_intr();

    return TRUE;
}

void r_sec(uint8_t dev_no, uint32_t sec_addr) {
    ide_wait(0);
    outb(DEVICE_CTRL_REG, 0);
    outb(SEC_COUNT_REG, 1);
    outb(SEC_NO_REG, sec_addr);
    outb(CYLINDER_LOW_REG, sec_addr >> 8u);
    outb(CYLINDER_HIGH_REG, sec_addr >> 16u);
    outb(DRIVE_HEAD_SEL_REG, 0xe0u | ((dev_no & 1u) << 4u) | (sec_addr >> 24u));
    outb(CMD_REG, READ);
}

void w_sec(uint8_t dev_no, uint32_t sec_addr, const void *src) {
    ide_wait(0);
    outb(DEVICE_CTRL_REG, 0);
    outb(SEC_COUNT_REG, 1);
    outb(SEC_NO_REG, sec_addr);
    outb(CYLINDER_LOW_REG, sec_addr >> 8u);
    outb(CYLINDER_HIGH_REG, sec_addr >> 16u);
    outb(DRIVE_HEAD_SEL_REG, 0xe0u | ((dev_no & 1u) << 4u) | (sec_addr >> 24u));
    outb(CMD_REG, WRITE);
    // write data
    while ((inb(ALTERNATE_STATUS_REG) & IDE_RDY) == 0);
    if (ide_wait(1) == 0 && (inb(ALTERNATE_STATUS_REG) & IDE_DRQ) != 0) {
        outsl(DATA_REG, src, SEC_SIZE / WORD_SIZE);
    }
}

static void ide_handler(__attribute__((unused)) intr_reg_t *intr_reg) {
    send_eoi(IDE_VEC);

    if ((inb(STATUS_REG) & (IDE_ERR | IDE_DF)) != 0) {
        // error
        return;
    }

    linked_list_node_t *tmp = dequeue(sec_queue);
    ide_sec_t *sec = (ide_sec_t *) tmp->data;
    release_bytes((uintptr_t) tmp);
    if (sec->op == FALSE) {
        // read from disk
        while ((inb(ALTERNATE_STATUS_REG) & IDE_RDY) == 0);
        insl(DATA_REG, (void *) sec->mem_addr, SEC_SIZE / WORD_SIZE);
        release_bytes((uintptr_t) sec);
    }
}

static int ide_wait(bool check_err) {
    uint8_t status;
    while (((status = inb(ALTERNATE_STATUS_REG)) & IDE_BSY) != 0) {
        if (check_err && (status & (IDE_DF | IDE_ERR)) != 0) {
            return -1;
        }
    }
    return 0;
}

void add_ide_task(ide_sec_t *ide_sec) {
    linked_list_node_t *new_node = (linked_list_node_t *) request_bytes(sizeof(linked_list_node_t));
    new_node->data = (uintptr_t) ide_sec;
    new_node->next = NULL;
    enqueue(sec_queue, new_node);
}

_Noreturn static void run_ide() {
    linked_list_node_t *new_node;
    while (TRUE) {
        while ((new_node = get_queue_head(sec_queue)) != NULL) {
            ide_sec_t *new_task = (ide_sec_t *) new_node->data;
            if (new_task->op) {
                w_sec(new_task->dev_no, new_task->sec_addr, (const void *) new_task->mem_addr);
            } else {
                r_sec(new_task->dev_no, new_task->sec_addr);
            }
        }
    }
}

void ide_write(uint8_t dev_no, uint32_t sec_addr, const void *src, uint32_t sec_num) {
    for (int i = 0; i < sec_num; ++i) {
        ide_sec_t *sec = (ide_sec_t *) request_bytes(sizeof(ide_sec_t));
        sec->op = TRUE;
        sec->mem_addr = (uintptr_t) src;
        sec->dev_no = dev_no;
        sec->sec_addr = sec_addr;
        add_ide_task(sec);
    }
}

void ide_read(uint8_t dev_no, uint32_t sec_addr, void *dst, uint32_t sec_num) {
    for (int i = 0; i < sec_num; ++i) {
        ide_sec_t *sec = (ide_sec_t *) request_bytes(sizeof(ide_sec_t));
        sec->op = FALSE;
        sec->mem_addr = (uintptr_t) dst;
        sec->dev_no = dev_no;
        sec->sec_addr = sec_addr;
        add_ide_task(sec);
    }
}