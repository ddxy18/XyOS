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

linked_list_node_t *get(linked_list_t list, pointer_t data) {
    linked_list_node_t *tmp = list.head;
    while (tmp->next != NULL) {
        if (tmp->next->data == data) {
            return tmp->next;
        }
    }
    return NULL;
}

/**
 * Remove 'remove_node' from 'list' and set 'remove_node.next' to 'NULL'. If 'remove_node' doesn't exist, return 'NULL'.
 * @param list
 * @param remove_node
 * @return Return 'remove_node' whose field 'next' is set to 'NULL'. If 'remove_node' doesn't exist, return 'NULL'.
 */
linked_list_node_t *remove(linked_list_t list, linked_list_node_t *remove_node) {
    linked_list_node_t *tmp = list.head;
    while (tmp->next != NULL) {
        if (tmp->next == remove_node) {
            tmp->next = remove_node->next;
            remove_node->next = NULL;
            return remove_node;
        }
    }
    return NULL;
}

#endif //XYOS_LIST_H
