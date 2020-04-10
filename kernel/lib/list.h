//
// Created by dxy on 2020/3/23.
//

/**
 * This is a list like the one heavily used in Linux kernel. It uses member 'list_node_t' in a struct to create a rounded linked list.
 * When we use the list, we must reserve a 'list_node_t' as a head and never do 'list_remove' to the head.
 * Firstly we have to call 'list_init(list_node_t *)' to initialize the list.
 * And we can use 'list_add(list_node_t *, list_node_t *)' to add new node.
 * Before we call 'list_remove(list_node_t *)' to remove a node, we must use 'is_list_empty(list_node_t *)' to determine whether there is any valid node.
 * Once we get the node we can use 'get_struct' to get the corresponding structure.
 */

#ifndef XYOS_LIST_H
#define XYOS_LIST_H

#include <stddef.h>

/**
 * Get struct address from its member address.
 *
 * @param ptr address of the member
 * @param type struct name
 * @param member member name in struct
 */
#define get_struct(ptr, type, member) ((type *)((char *)(ptr)-(unsigned long)(&((type *)0)->member)))

typedef struct list_node {
    struct list_node *prev, *next;
} list_node_t;

static inline list_node_t *list_init(list_node_t *head) {
    head->prev = head;
    head->next = head;
}

static inline void list_add(list_node_t *prev, list_node_t *new_node) {
    list_node_t *next = prev->next;
    prev->next = new_node;
    new_node->prev = prev;
    new_node->next = next;
    next->prev = new_node;
}

static inline list_node_t *list_remove(list_node_t *node) {
    node->prev->next = node->next;
    node->next->prev = node->prev;
    node->prev = NULL;
    node->next = NULL;
    return node;
}

static inline bool is_list_empty(list_node_t *head) {
    if (head->next == head) {
        return TRUE;
    }
    return FALSE;
}

#endif // XYOS_LIST_H