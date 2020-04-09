//
// Created by dxy on 2020/4/8.
//

#ifndef XYOS_IDE_H
#define XYOS_IDE_H

#include <stdint.h>
#include "../lib/def.h"

// Every sector in our drive is 512 bytes.
#define SEC_SIZE 512

// Now we only use a drive of index 1.
#define DEV_NO 1

/**
 *
 * @return bool Return 'TRUE' if at least a drive exists. Otherwise return 'FALSE'.
 */
bool ide_init();

/**
 * Start transfer between disk and memory.
 */
_Noreturn static void run_ide();

void ide_read(uint8_t dev_no, uint32_t sec_addr, void *dst, uint32_t sec_num);

void ide_write(uint8_t dev_no, uint32_t sec_addr, const void *src, uint32_t sec_num);

#endif //XYOS_IDE_H