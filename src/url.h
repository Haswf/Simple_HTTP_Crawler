//
// Created by Haswe on 3/31/2020.
//

#ifndef COMP30023_2020_PROJECT1_URL_H
#define COMP30023_2020_PROJECT1_URL_H

#define SCHEME_INDEX 2
#define AUTHORITY_INDEX 4
#define PATH_INDEX 5
#define QUERY_INDEX 7
#define FRAGMENT_INDEX 8

#include <regex.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "../lib/log/log.h"
#include "config.h"
#include "collection.h"

/**
 * Structure to store components of a URI
 */
typedef struct url {
    sds scheme;
    sds authority;
    sds path;
    sds query;
    sds fragment;
    sds raw;
} url_t;


int free_url(url_t *url);

url_t *parse_url(sds text);

sds remove_dot_segment(sds input);

sds merge_path(sds base_uri, sds relative_path);

sds recomposition(url_t *url);

url_t *resolve_reference(sds reference, sds base);

sds safe_sdsdup(sds toCopy);

bool is_valid_url(sds url);

#endif //COMP30023_2020_PROJECT1_URL_H
