/**
 * Module to define data structures used in the project.
 * Created by Shuyang Fan on 3/25/2020.
 */

#ifndef COMP30023_2020_PROJECT1_COLLECTION_H
#define COMP30023_2020_PROJECT1_COLLECTION_H

/**
 * External library usage: vec
 * vec is a type-safe dynamic array implementation written by rxi.
 * The source code is under MIT license and can be obtained at https://github.com/rxi/vec
 */
#include "../lib/vec/vec.h"

/**
 * External library usage: sds
 * sds is a dynamic string library implemented by Salvatore Sanfilippo
 * The source code is licensed under BSD 2-Clause "Simplified",
 * which can be obtained at https://github.com/antirez/sds
 */
#include "../lib/sds/sds.h"

/**
 * External library usage: map
 * map is a map library implemented by rxi
 * The source code is under MIT license and can be obtained at https://github.com/rxi/map
 */
#include "../lib/map/map.h"

/* Map<String, int> */
typedef map_t(int) int_map_t;
/* List<String> */
typedef vec_t(sds) sds_vec_t;
/* Map<String, String> */
typedef map_t(sds) sds_map_t;

int free_sds_map(sds_map_t **map);

#endif //COMP30023_2020_PROJECT1_COLLECTION_H
