/**
 * Module to define data structures used in the .
 * Created by Shuyang Fan on 3/25/2020.
 */

#ifndef COMP30023_2020_PROJECT1_COLLECTION_H
#define COMP30023_2020_PROJECT1_COLLECTION_H

#include "../lib/vec/vec.h"
#include "../lib/sds/sds.h"
#include "../lib/map/map.h"

typedef map_t(int) int_map_t;
typedef vec_t(sds) sds_vec_t;
typedef map_t(sds) sds_map_t;

#endif //COMP30023_2020_PROJECT1_COLLECTION_H
