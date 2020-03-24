//
// Created by Haswe on 3/23/2020.
//

#ifndef COMP30023_2020_PROJECT1_RESPONSE_H
#define COMP30023_2020_PROJECT1_RESPONSE_H

#include "../lib/sds/sds.h"
#include "../lib/map/map.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "./request.h"
//typedef map_t(sds) sds_map_t;

typedef struct Response {
    sds version;
    int status_code;
    sds reason_phrase;
    sds body;
    sds_map_t *header;
} Response;

Response *parse_response(sds *buffer);

void print_header(Response *response);

void print_body(Response *response);

void free_response(Response *response);

#endif //COMP30023_2020_PROJECT1_RESPONSE_H
