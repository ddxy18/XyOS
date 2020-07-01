//
// Created by dxy on 2020/4/11.
//

#ifndef XYOS_FILE_H
#define XYOS_FILE_H

#include "fs.h"

// temporary data structure
typedef struct dev_inode {

} dev_inode_t;

typedef struct mem_inode {
    union {
        file_tree_node_t *file_tree_inode;
        dev_inode_t *dev_inode;
    } inode;
    unsigned int ref;
    unsigned int modified;
} mem_inode_t;

typedef struct file {
    mem_inode_t *inode;
    unsigned int cur;
} file_t;

#endif //XYOS_FILE_H
