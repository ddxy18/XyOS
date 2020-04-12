//
// Created by dxy on 2020/3/23.
//

/**
 * This file contains two types of list: circular linked list and doubly linked list.
 * User should add a member 'doubly_list_node_t' or 'list_node_t' in a struct to create a circular linked list of the
 * struct. When we use the list, we must reserve a 'doubly_list_node_t' or a 'list_node_t' as a head and never do
 * 'list_remove' to the head.
 * In this file, we provide 'doubly_list()' and 'list()' to create the head. And when we need to destruct the list, we
 * must destruct all nodes exclude head node ourselves and then call 'destruct_list(list_node_t *)' or
 * 'destruct_doubly_list(doubly_list_node_t *)' to destruct the head.
 * We can use 'doubly_list_add(doubly_list_node_t *, doubly_list_node_t *)' or 'list_add(list_node_t *, list_node_t *)'
 * to add new node.
 * To remove a node, we can use 'list_remove(list_node_t *, list_node_t *)' or 'doubly_list_remove(doubly_list_node_t *)'.
 * Once we get the node we can use 'get_struct' to get the corresponding structure. Notice that before we call the two
 * function to remove a node, we must use 'is_list_empty(list_node_t *)' or 'is_doubly_list_empty(doubly_list_node_t *)'
 * to determine whether there is any valid node and we should ensure that we don't remove the head node.
 *
 * A list should be used as belows:
 *
 *  // create an empty list
 *  list_node_t *head = list();
 *  ...
 *
 *  // add new node
 *  // initialize 'new_node' and the struct it belongs to yourself
 *  ...
 *  list_add(new_node);
 *  ...
 *
 *  // remove node
 *  // 'prev' should never be 'head'
 *  If (is_list_empty(prev) == FALSE) {
 *      list_remove(prev, cur);
 *  }
 *  // may be need to destruct 'cur' and the struct it belongs to
 *  ...
 *
 *  // destruct list
 *  while (is_list_empty(head) == FALSE) {
 *      // may be need to destruct the struct 'head->next' belongs to
 *      ...
 *      list_remove(head->next);
 *  }
 *  destruct_list(head);
 */

#ifndef XYOS_LIST_H
#define XYOS_LIST_H

#include <stddef.h>
#include "def.h"
#include "../memory/direct_mapping_allocator.h"

/**
 * Get struct address from its member address.
 *
 * @param ptr address of the member
 * @param type struct name
 * @param member member name in the struct
 */
#define get_struct(ptr, type, member) ((type *)((char *)(ptr)-(unsigned long)(&((type *)0)->member)))

/// @def list_node_t a circular linked list
typedef struct list_node {
    struct list_node *next;
} list_node_t;

/// @def doubly_list_node_t a doubly circular linked list
typedef struct doubly_list_node {
    struct doubly_list_node *prev, *next;
} doubly_list_node_t;

static inline doubly_list_node_t *doubly_list() {
    doubly_list_node_t *head = (doubly_list_node_t *) request_bytes(sizeof(doubly_list_node_t));
    if (head == NULL) {
        return NULL;
    }
    head->prev = head;
    head->next = head;
    return head;
}

static inline void doubly_list_add(doubly_list_node_t *prev, doubly_list_node_t *new_node) {
    doubly_list_node_t *next = prev->next;
    prev->next = new_node;
    new_node->prev = prev;
    new_node->next = next;
    next->prev = new_node;
}

static inline doubly_list_node_t *doubly_list_remove(doubly_list_node_t *node) {
    node->prev->next = node->next;
    node->next->prev = node->prev;
    node->prev = NULL;
    node->next = NULL;
    return node;
}

static inline bool is_doubly_list_empty(doubly_list_node_t *head) {
    if (head->next == head) {
        return TRUE;
    }
    return FALSE;
}

static inline void destruct_doubly_list(doubly_list_node_t *head) {
    release_bytes((uintptr_t) head);
}

static inline list_node_t *list() {
    list_node_t *head = (list_node_t *) request_bytes(sizeof(list_node_t));
    if (head == NULL) {
        return NULL;
    }
    head->next = head;
    return head;
}

static inline void list_add(list_node_t *prev, list_node_t *new_node) {
    list_node_t *next = prev->next;
    prev->next = new_node;
    new_node->next = next;
}

static inline list_node_t *list_remove(list_node_t *prev, list_node_t *node) {
    prev->next = node->next;
    node->next = NULL;
    return node;
}

static inline bool is_list_empty(list_node_t *head) {
    if (head->next == head) {
        return TRUE;
    }
    return FALSE;
}

static inline void destruct_list(list_node_t *head) {
    release_bytes((uintptr_t) head);
}

#endif //XYOS_LIST_H