//
// Created by dxy on 2020/4/8.
//

#include "queue.h"
#include "../memory/direct_mapping_allocator.h"

k_queue_t *queue() {
    k_queue_t *new_queue = (k_queue_t *) request_bytes(sizeof(k_queue_t));
    new_queue->head = (linked_list_node_t *) request_bytes(sizeof(linked_list_node_t));
    new_queue->tail = new_queue->head;
    return new_queue;
}

void enqueue(k_queue_t *queue, linked_list_node_t *new_node) {
    new_node->next = NULL;
    queue->tail->next = new_node;
    queue->tail = new_node;
}

linked_list_node_t *dequeue(k_queue_t *queue) {
    if (queue->head == queue->tail) {
        // empty queue
        return NULL;
    }

    linked_list_node_t *tmp = queue->head->next;
    queue->head->next = tmp->next;

    if (queue->head->next == NULL) {
        // will become an empty queue
        queue->tail = queue->head;
    }

    tmp->next = NULL;
    return tmp;
}

linked_list_node_t *get_queue_head(k_queue_t *queue) {
    return queue->head->next;
}