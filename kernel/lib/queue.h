//
// Created by dxy on 2020/4/8.
//

#ifndef XYOS_QUEUE_H
#define XYOS_QUEUE_H

#include "list.h"

typedef struct k_queue {
    linked_list_node_t *head;
    linked_list_node_t *tail;
} k_queue_t;

k_queue_t *queue();

void enqueue(k_queue_t *queue, linked_list_node_t *new_node);

linked_list_node_t *dequeue(k_queue_t *queue);

linked_list_node_t *get_queue_head(k_queue_t *queue);

#endif //XYOS_QUEUE_H
