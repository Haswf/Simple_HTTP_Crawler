//
// Created by Haswe on 3/31/2020.
//

#ifndef COMP30023_2020_PROJECT1_URL_H
#define COMP30023_2020_PROJECT1_URL_H

#include "../lib/sds/sds.h"

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

#endif //COMP30023_2020_PROJECT1_URL_H
