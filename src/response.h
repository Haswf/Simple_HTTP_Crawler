//
// Created by Haswe on 3/23/2020.
//

#ifndef COMP30023_2020_PROJECT1_RESPONSE_H
#define COMP30023_2020_PROJECT1_RESPONSE_H


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "connection.h"
#include "collection.h"
#include <ctype.h>

typedef struct response {
    sds version;
    int status_code;
    sds reason_phrase;
    sds body;
    sds_map_t *header;
} response_t;

response_t *parse_response(sds *buffer);

void print_header(response_t *response);

void print_body(response_t *response);

void free_response(response_t *response);

sds lower(sds string);

#endif //COMP30023_2020_PROJECT1_RESPONSE_H
