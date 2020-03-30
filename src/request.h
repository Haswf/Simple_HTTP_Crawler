//
// Created by Haswell on 3/21/2020.
//

#ifndef COMP30023_2020_PROJECT1_REQUEST_H
#define COMP30023_2020_PROJECT1_REQUEST_H

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "collection.h"
#include "config.h"
#include "url.h"

typedef struct Request {
    sds method;
    sds host;
    sds path;
    url_t *parsed_url;
    sds version;
    sds body;
    sds_map_t *header;
} Request;


sds HTTPRequestToString(Request *);

Request *create_http_request(sds host, sds path, sds method, sds body);

int add_header(Request *req, char *name, char *value);

int free_request(Request *req);
#endif