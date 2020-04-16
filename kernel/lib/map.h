//
// Created by dxy on 2020/4/5.
//

#ifndef XYOS_MAP_H
#define XYOS_MAP_H

#include <stdint.h>

#include "list.h"

typedef struct MapEntry {
    /**
     * 'key' is 'unsigned char' type, so a 'map_t' structure can have 255
     * entries at most.
     */
    unsigned char key;
    void *value;
    ListNode node;
} MapEntry;

typedef ListNode Map;

static inline Map *map() {
    return List();
}

static inline void MapAdd(Map *map, MapEntry *entry) {
    ListAdd(map, &entry->node);
}

/**
 * @param map
 * @param key
 * @return Value corresponding to the 'key'. If the requested key doesn't exist,
 * it will
 * return 'NULL'.
 */
static inline void *MapGet(Map *map, unsigned char key) {
    ListNode *node = map;
    while (node->next != map) {
        if (GET_STRUCT(node->next, MapEntry, node)->key == key) {
            return GET_STRUCT(ListRemove(node), MapEntry, node)->value;
        }
    }

    return NULL;
}

/**
 *
 * @param map
 * @param key
 * @return Removed 'MapEntry'. If the requested key doesn't exist, it will
 * return 'NULL'.
 */
static inline MapEntry *MapRemove(Map *map, unsigned char key) {
    ListNode *node = map;
    while (node->next != map) {
        if (GET_STRUCT(node->next, MapEntry, node)->key == key) {
            return GET_STRUCT(ListRemove(node), MapEntry, node);
        }
    }

    return NULL;
}

static inline bool IsMapEmpty(Map *map) {
    return IsListEmpty(map);
}

#endif //XYOS_MAP_H