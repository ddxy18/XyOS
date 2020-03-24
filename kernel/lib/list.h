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

void insert_to_head(linked_list_t list, linked_list_node_t *new_node) {
    linked_list_node_t *tmp = list.head;
    new_node->next = tmp->next;
    tmp->next = new_node;
}

#endif //XYOS_LIST_H
