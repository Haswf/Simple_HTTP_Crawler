//
// Created by Haswe on 3/25/2020.
//

#ifndef COMP30023_2020_PROJECT1_PARSER_H
#define COMP30023_2020_PROJECT1_PARSER_H


#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "../lib/gumbo/gumbo.h"
#include "response.h"
#include "../lib/log/log.h"
#include "request.h"

void print_url(Response *response);

void search_and_add_url(url_t *url_parse, sds html, sds_vec_t *job_queue, int_map_t *seen);

//void add_url(sds html, Request* request, sds_vec_t *job_queue, int_map_t *seen);
void add_to_job_queue(url_t *url_parse, GumboNode *node, sds_vec_t *job_queue, int_map_t *seen);

sds resolve_referencing(sds relative_path, sds base_url);

#endif //COMP30023_2020_PROJECT1_PARSER_H
