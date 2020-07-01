//
// Created by dxy on 2020/4/11.
//

#ifndef XYOS_FS_H
#define XYOS_FS_H

#include <stdint.h>
#include "../lib//list.h"

#define MAX_BLOCK_NUM 0x10000

// persistence data structure
typedef struct block_list {
    unsigned int block_addr;
    ListNode *node;
} block_list_t;

typedef struct drive_inode {
    unsigned int size;
    block_list_t *head;
    // read-only or read-write
    unsigned int type;
} drive_inode_t;

typedef struct file_tree_node {
    char *name;
    // 'NULL' means directory
    drive_inode_t *drive_inode;
    ListNode left;
    ListNode right;
} file_tree_node_t;

void FileTreeInit();

#endif //XYOS_FS_H
