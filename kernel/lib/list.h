//
// Created by dxy on 2020/3/23.
//

/**
 * This file contains two types of list: circular linked list and doubly
 * linked list.
 * User should add a member 'DoublyListNode' or 'ListNode' in a struct to
 * create a circular linked List of the struct. When we use the List, we must
 * reserve a 'DoublyListNode' or a 'ListNode' as a head and never call
 * 'ListRemove' to the head.
 * In this file, we provide 'DoublyList' and 'List' to create the head. And
 * when we need to destruct the List, we must destruct all nodes exclude head
 * node ourselves and then call 'DestructList' or 'DestructDoublyList' to
 * destruct the head.
 * We can use 'DoublyListAdd' or 'ListAdd' to add new node.
 * To remove a node, we can use 'ListRemove' or 'DoublyListRemove'. Once we
 * get the node we can use 'GET_STRUCT' to get the corresponding structure.
 * Notice that before we call either function to remove a node, we must use
 * 'IsListEmpty' or 'IsDoublyListEmpty' to determine whether there is any
 * valid node so we can ensure that we don't remove the head node.
 *
 * A List should be used as belows:
 *
 *  // create an empty List
 *  ListNode *head = List();
 *  ...
 *
 *  // add new node
 *  // initialize 'new_node' and the struct it belongs to yourself
 *  ...
 *  ListAdd(new_node);
 *  ...
 *
 *  // remove node
 *  // 'prev' should never be 'head'
 *  If (IsListEmpty(prev) == FALSE) {
 *      ListRemove(prev);
 *  }
 *  // may be need to destruct 'cur' and the struct it belongs to
 *  ...
 *
 *  // destruct List
 *  while (IsListEmpty(head) == FALSE) {
 *      // may be need to destruct the struct 'head->next' belongs to
 *      ...
 *      ListRemove(head);
 *  }
 *  DestructList(head);
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
#define GET_STRUCT(ptr, type, member) ((type *)((char *)(ptr)-(unsigned long)(&((type *)0)->member)))

/// @typedef ListNode a circular linked List
typedef struct ListNode {
    struct ListNode *next;
} ListNode;

// @typedef DoublyListNode a doubly circular linked List
typedef struct DoublyListNode {
    struct DoublyListNode *prev, *next;
} DoublyListNode;

static inline DoublyListNode *DoublyList() {
    DoublyListNode *head = (DoublyListNode *) ReqBytes(sizeof(DoublyListNode));
    if (head == NULL) {
        return NULL;
    }
    head->prev = head;
    head->next = head;
    return head;
}

static inline void
DoublyListAdd(DoublyListNode *prev, DoublyListNode *new_node) {
    DoublyListNode *next = prev->next;
    prev->next = new_node;
    new_node->prev = prev;
    new_node->next = next;
    next->prev = new_node;
}

static inline DoublyListNode *DoublyListRemove(DoublyListNode *node) {
    node->prev->next = node->next;
    node->next->prev = node->prev;
    node->prev = NULL;
    node->next = NULL;
    return node;
}

static inline bool IsDoublyListEmpty(DoublyListNode *head) {
    if (head->next == head) {
        return TRUE;
    }
    return FALSE;
}

static inline void DestructDoublyList(DoublyListNode *head) {
    RelBytes((uintptr_t) head);
}

static inline ListNode *List() {
    ListNode *head = (ListNode *) ReqBytes(sizeof(ListNode));
    if (head == NULL) {
        return NULL;
    }
    head->next = head;
    return head;
}

static inline void ListAdd(ListNode *prev, ListNode *new_node) {
    ListNode *next = prev->next;
    prev->next = new_node;
    new_node->next = next;
}

static inline ListNode *ListRemove(ListNode *prev) {
    ListNode *del = prev->next;
    prev->next = del->next;
    del->next = NULL;
    return del;
}

static inline bool IsListEmpty(ListNode *head) {
    if (head->next == head) {
        return TRUE;
    }
    return FALSE;
}

static inline void DestructList(ListNode *head) {
    RelBytes((uintptr_t) head);
}

#endif //XYOS_LIST_H