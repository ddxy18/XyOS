//
// Created by dxy on 2020/4/5.
//

#include "map.h"
#include "../memory/direct_mapping_allocator.h"

map_entry_t *map_init(map_entry_t *head) {
    head->node = (list_node_t *) request_bytes(sizeof(list_node_t));
    list_init(head->node);
}

void map_add(map_entry_t *prev, map_entry_t *entry) {
    entry->node = (list_node_t *) request_bytes(sizeof(list_node_t));
    list_add(prev->node, entry->node);
}

map_entry_t *map_get(map_entry_t *head, unsigned char key) {
    list_node_t *tmp = head->node->next;

    map_entry_t *map_entry;
    while (tmp != head->node) {
        map_entry = get_struct(tmp, map_entry_t, node);
        if (map_entry->key == key) {
            return map_entry;
        }
        tmp = tmp->next;
    }
    return NULL;
}

map_entry_t *map_remove(map_entry_t *head, unsigned char key) {
    map_entry_t *map_entry = map_get(head, key);
    release_bytes((uintptr_t) list_remove(map_entry->node));
    map_entry->node = NULL;
    return map_entry;
}
