//
// Created by dxy on 2020/4/8.
//

/**
 * This is a queue based on the circular linked list structure in 'list.h'.
 * User should reserve a 'Queue' structure to operate the queue by calling
 * the function in this file. Notice that user had better use the functions
 * below through the whole lifetime of the queue. Modifying the queue
 * directly through the circular linked List may destroy the queue and
 * calling functions below after that may lead to unpredicted errors.
 *
 * A queue should be used as below:
 *
 *  // create an empty queue
 *  Queue *q = queue();
 *  ...
 *
 *  // enqueue
 *  Enqueue(q, new_node);
 *  ...
 *
 *  // dequeue
 *  // user should check whether 'node' is empty
 *  ListNode *node = Dequeue(q);
 *  // maybe need to destruct 'node' and the struct it belongs to
 *  ...
 *
 *  // get queue head but doesn't remove it
 *  ListNode *head = QueueGetHead(q);
 *  // user should check whether 'head' is empty
 *  ...
 *
 *  // destruct queue
 *  ListNode *del;
 *  while ((del = Dequeue(q)) != NULL) {
 *      // may be need to destruct 'del' and the struct it belongs to
 *      ...
 *  }
 *  DestructQueue(q);
 */

#ifndef XYOS_QUEUE_H
#define XYOS_QUEUE_H

#include "list.h"

/**
 * @typedef Queue 'head' points to the head in the circular linked List and
 * 'tail' points to the node before 'head'.
 */
typedef struct Queue {
    ListNode *head;
    ListNode *tail;
} Queue;

static inline Queue *queue() {
    ListNode *head = List();
    Queue *q = (Queue *) ReqBytes(sizeof(Queue));
    q->head = head;
    q->tail = head;
    return q;
}

static inline void Enqueue(Queue *queue, ListNode *new_node) {
    ListAdd(queue->tail, new_node);
    queue->tail = new_node;
}

static inline ListNode *Dequeue(Queue *queue) {
    if (IsListEmpty(queue->head) == TRUE) {
        // empty queue
        return NULL;
    }

    if (queue->head->next == queue->tail) {
        queue->tail = queue->head;
    }
    return ListRemove(queue->head);
}

static inline ListNode *QueueGetHead(Queue *queue) {
    if (IsListEmpty(queue->head) == TRUE) {
        return NULL;
    }
    return queue->head->next;
}

static inline void DestructQueue(Queue *queue) {
    DestructList(queue->head);
    RelBytes((uintptr_t) queue);
}

#endif //XYOS_QUEUE_H