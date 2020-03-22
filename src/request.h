//
// Created by Haswell on 3/21/2020.
//

#ifndef COMP30023_2020_PROJECT1_REQUEST_H
#define COMP30023_2020_PROJECT1_REQUEST_H

#include <stdio.h>
#include <stdlib.h>
#include "../lib/sds.h"
#include "../lib/vec.h"

typedef struct Header {
    sds name;
    sds value;
} Header;

/* Creates the type uint_vec_t for storing unsigned ints */
typedef vec_t(Header*) vec_header_t;

typedef struct Request {
    sds method;
    sds host;
    sds path;
    sds version;
    sds body;
    vec_header_t *headers;
} Request;

sds HTTPRequestToString(Request *);

Request *createHTTPRequest(sds host, sds path, sds method, vec_header_t *vec_header_p, sds body);

Header *createHeader(sds name, sds value);

int appendHeader(Request *req, char *name, char *value);

int freeRequest(Request *req);
#endif