//
// Created by dxy on 2020/4/11.
//

/**
 * block 0 as super block
 */

#include "fs.h"

#include "../driver/ide.h"

uintptr_t *kSuperBlock;

void FileTreeInit() {
    kSuperBlock = (uintptr_t *) ReqBytes(kSecSize * kBlockSize);
    // load super block
    IdeRead(1, 0, kSuperBlock, 1);
}