//
// Created by dxy on 2020/3/24.
//

#include <stddef.h>
#include <stdint.h>

#include "boot/hardware.h"
#include "boot/multiboot.h"
#include "driver/console.h"
#include "driver/driver.h"
#include "interrupt/interrupt.h"
#include "memory/memory_manager.h"
#include "lib/bit.h"
#include "process/process.h"
#include "thread/thread.h"
#include "syscall/syscall.h"

/**
 * @param magic the 32-bit value indicates whether bootloader complies Multiboot
 * standard
 * @param multiboot_info physical address of Multiboot information structure
 * @return nonzero when meets failure
 */
int kernel_start(uint32_t magic, multiboot_info_t *multiboot_info) {

    video = (volatile unsigned char *) VIDEO;
    cls();

    /**
     * detect magic to determine whether loaded by Multiboot-compliant
     * bootloader
     */
    if (magic != MULTIBOOT_BOOTLOADER_MAGIC) {
        printf("Invalid magic number: 0x%x\n", (unsigned) magic);
        return -1;
    }

    // detect memory size
    if (IsBitSet(multiboot_info->flags, 0)) {
        if (multiboot_info->mem_lower < MIN_LOWER_MEM ||
            multiboot_info->mem_upper < MIN_UPPER_MEM) {
            // not enough memory
            return -1;
        }
    }
    // detect available memory size and physical address
    if (IsBitSet(multiboot_info->flags, 6)) {
        AvlPhysMem *t = GetAvlMem();

        multiboot_memory_map_t *mmap =
                (multiboot_memory_map_t *) multiboot_info->mmap_addr;
        for (; (unsigned long) mmap <
               multiboot_info->mmap_addr + multiboot_info->mmap_length;
               mmap = (multiboot_memory_map_t *) ((unsigned long) mmap +
                                                  mmap->size +
                                                  sizeof(mmap->size))) {
            if (mmap->type == MULTIBOOT_MEMORY_AVAILABLE) {
                t->addr = mmap->addr;
                t->size = mmap->len;
                t++;
            }
        }
        t->size = UINT32_MAX;
        t = NULL;
    }

    printf("Initialize memory management...\n");
    MemInit();
    printf("Complete.\n");

    // set new IDT
    printf("Initialize interrupt...\n");
    IntrInit();
    printf("Complete.\n");
    printf("Register syscall...\n");
    SyscallInit();
    printf("Complete.\n");
    printf("Initialize process manager...\n");
    ProcManagerInit();
    printf("Complete.\n");
    printf("Initialize thread...\n");
    ThreadInit();
    printf("Complete.\n");
    printf("Initialize driver...\n");
    DriverInit();
    printf("Complete.\n");
    asm("sti");
}