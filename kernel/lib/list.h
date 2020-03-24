//
// Created by dxy on 2020/3/23.
//

#ifndef XYOS_LIST_H
#define XYOS_LIST_H

#include <stdint.h>
#include "def.h"

typedef struct linked_list_node {
    pointer_t data;
    struct linked_list_node *next;
} linked_list_node_t;

typedef struct linked_list {
    linked_list_node_t *head;
} linked_list_t;

#endif //XYOS_LIST_H
