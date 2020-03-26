//
// Created by Haswe on 3/25/2020.
//

#ifndef COMP30023_2020_PROJECT1_PARSER_H
#define COMP30023_2020_PROJECT1_PARSER_H


#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "../lib/url_parser/url_parser.h"
#include "../lib/gumbo/gumbo.h"
#include "response.h"
#include "../lib/uriparser/Uri.h"
#include "../lib/uriparser/UriBase.h"
#include "../lib/uriparser/UriBase.h"
#include "../lib/uriparser/UriDefsAnsi.h"
#include "../lib/uriparser/UriDefsConfig.h"
#include "../lib/uriparser/UriDefsUnicode.h"
#include "../lib/uriparser/UriIp4.h"
#include "../lib/log/log.h"
#include "request.h"

void print_url(Response *response);

void add_url(Request *request, sds html, sds_vec_t *job_queue, int_map_t *seen);

//void add_url(sds html, Request* request, sds_vec_t *job_queue, int_map_t *seen);
void add_to_job_queue(sds host, GumboNode *node, sds_vec_t *job_queue, int_map_t *seen);

#endif //COMP30023_2020_PROJECT1_PARSER_H
