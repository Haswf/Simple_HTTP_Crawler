//
// Created by Haswe on 4/6/2020.
//

#include "collection.h"
#include "config.h"

int free_sds_map(sds_map_t **map) {
    if (*map) {
        const char *key;
        map_iter_t iter = map_iter(*map);

        while ((key = map_next(*map, &iter))) {
            sdsfree(*map_get(*map, key));
        }
        map_deinit(*map);
        free(*map);
        *map = NULL;
        return SUCCESS;
    }
    return ERROR;
}

