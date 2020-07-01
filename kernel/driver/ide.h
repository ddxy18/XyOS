//
// Created by dxy on 2020/4/8.
//

#ifndef XYOS_IDE_H
#define XYOS_IDE_H

#include <stdint.h>

#include "../lib/def.h"

// Every sector in our drive is 512 bytes.
static const int kSecSize = 512;
// how many sectors a block have
static const int kBlockSize = 8;

/**
 *
 * @return bool Return 'TRUE' if at least a drive exists. Otherwise return
 * 'FALSE'.
 */
bool IdeInit();

void IdeRead(uint8_t dev_no, uint32_t sec_addr, void *dst, uint32_t block_num);

void IdeWrite(uint8_t dev_no, uint32_t sec_addr, const void *src, uint32_t
block_num);

#endif //XYOS_IDE_H