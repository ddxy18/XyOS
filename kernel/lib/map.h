//
// Created by dxy on 2020/4/5.
//

#ifndef XYOS_MAP_H
#define XYOS_MAP_H

#include <stdint.h>
#include "list.h"

typedef struct map_entry {
    // 'key' is 'unsigned char' type, so a 'map_t' structure can have 255 entries at most.
    unsigned char key;
    list_node_t *node;
} map_entry_t;

map_entry_t *map_init(map_entry_t *head);

void map_add(map_entry_t *prev, map_entry_t *entry);

/**
 * @param head
 * @param key
 * @return Value's address. If the requested key doesn't exist, it will return 'NULL'.
 */
map_entry_t *map_get(map_entry_t *head, unsigned char key);

map_entry_t *map_remove(map_entry_t *head, unsigned char key);

#endif //XYOS_MAP_H
