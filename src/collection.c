//
// Created by Haswe on 4/6/2020.
//
#include "collection.h"

void free_map(sds_map_t **map) {
    const char *key;
    map_iter_t iter = map_iter(*map);

    while ((key = map_next(*map, &iter))) {
        sdsfree(*map_get(*map, key));
    }
    map_deinit(*map);
    free(*map);
    *map = NULL;
}