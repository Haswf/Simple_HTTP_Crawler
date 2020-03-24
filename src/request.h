//
// Created by Haswell on 3/21/2020.
//

#ifndef COMP30023_2020_PROJECT1_REQUEST_H
#define COMP30023_2020_PROJECT1_REQUEST_H

#include <stdio.h>
#include <stdlib.h>
#include "../lib/sds/sds.h"
#include "../lib/vec/vec.h"
#include "../lib/map/map.h"

/* Creates the type uint_vec_t for storing unsigned ints */
typedef map_t(sds) sds_map_t;

typedef struct Request {
    sds method;
    sds host;
    sds path;
    sds version;
    sds body;
    sds_map_t *header;
} Request;


sds HTTPRequestToString(Request *);

Request *create_http_request(sds host, sds path, sds method, sds body);

int add_header(Request *req, char *name, char *value);

int free_request(Request *req);
#endif