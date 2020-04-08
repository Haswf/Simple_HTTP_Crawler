/**
 * Module for data structures used in the project.
 * Created by Shuyang Fan on 3/25/2020.
 */

#include "collection.h"
#include "config.h"

/**
 * Free a sds_map and its stored value.
 * @param map
 * @return
 */
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

