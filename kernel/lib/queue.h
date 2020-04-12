//
// Created by dxy on 2020/4/8.
//

/**
 * This is a queue based on the circular linked list structure in 'list.h'.
 * User should reserve a 'queue_t' structure to operate the queue by calling the function in this file. Notice that user
 * had better use the functions below through the whole lifetime of the queue. Modifying the queue directly through the
 * circular linked list may destroy the queue and calling functions below after that may lead to unpredicted errors.
 *
 * A queue should be used as below:
 *
 *  // create an empty queue
 *  queue_t *q = queue();
 *  ...
 *
 *  // enqueue
 *  enqueue(q, new_node);
 *  ...
 *
 *  // dequeue
 *  // user should check whether 'node' is empty
 *  list_node_t *node = dequeue(q);
 *  // maybe need to destruct 'node' and the struct it belongs to
 *  ...
 *
 *  // get queue head but doesn't remove it
 *  list_node_t *head = queue_get_head(q);
 *  // user should check whether 'head' is empty
 *  ...
 *
 *  // destruct queue
 *  list_node_t *del;
 *  while ((del = dequeue(q)) != NULL) {
 *      // may be need to destruct 'del' and the struct it belongs to
 *      ...
 *  }
 *  destruct_queue(q);
 */

#ifndef XYOS_QUEUE_H
#define XYOS_QUEUE_H

#include "list.h"

/// @def queue_t 'head' points to the head in the circular linked list and 'tail' points to the node before 'head'.
typedef struct queue {
    list_node_t *head;
    list_node_t *tail;
} queue_t;

static inline queue_t *queue() {
    list_node_t *head = list();
    queue_t *q = (queue_t *) request_bytes(sizeof(queue_t));
    q->head = head;
    q->tail = head;
    return q;
}

static inline void enqueue(queue_t *queue, list_node_t *new_node) {
    list_add(queue->tail, new_node);
    queue->tail = new_node;
}

static inline list_node_t *dequeue(queue_t *queue) {
    if (is_list_empty(queue->head) == TRUE) {
        // empty queue
        return NULL;
    }

    if (queue->head->next == queue->tail) {
        queue->tail = queue->head;
    }
    return list_remove(queue->head, queue->head->next);
}

static inline list_node_t *queue_get_head(queue_t *queue) {
    if (is_list_empty(queue->head) == TRUE) {
        return NULL;
    }
    return queue->head->next;
}

static inline void destruct_queue(queue_t *queue) {
    destruct_list(queue->head);
    release_bytes((uintptr_t) queue);
}

#endif //XYOS_QUEUE_H