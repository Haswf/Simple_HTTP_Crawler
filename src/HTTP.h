//
// Created by Haswe on 3/22/2020.
//

#ifndef COMP30023_2020_PROJECT1_HTTP_H
#define COMP30023_2020_PROJECT1_HTTP_H

#include <stdio.h> /* printf, sprintf */
#include <stdlib.h> /* exit, atoi, malloc, free */
#include "request.h"
#include "response.h"

Response *send_http_request(Request *request, int portno, int *status);

char *locate_body(char *buffer);

sds_map_t *extract_header(char *buffer);

bool isHTML(sds_map_t *header_map);

bool isBufferSufficient(sds_map_t *header_map);

bool parse_int(sds string, int *parse_result);

#endif //COMP30023_2020_PROJECT1_HTTP_H
