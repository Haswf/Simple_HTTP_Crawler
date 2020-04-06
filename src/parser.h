//
// Created by Haswe on 3/25/2020.
//

#ifndef COMP30023_2020_PROJECT1_PARSER_H
#define COMP30023_2020_PROJECT1_PARSER_H


#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "request.h"
#include "response.h"
#include "url.h"
/**
 * External library usage: log
 * log is a logging library implemented in C99 written by rxi.
 * The source code is under MIT license and can be obtained at https://github.com/rxi/log.c
 */
#include "../lib/log/log.h"

/**
 * External library usage: gumbo
 * gumbo is an HTML5 parsing library in pure C99 developed by Google
 * The source code is under Apache License 2.0 and can be obtained at https://github.com/google/gumbo-parser
 */
#include "../lib/gumbo/gumbo.h"

void search_and_add_url(url_t *url_parse, sds html, sds_vec_t *job_queue, int_map_t *seen);

void add_to_job_queue(url_t *url_parse, GumboNode *node, sds_vec_t *job_queue, int_map_t *seen);

sds getLocation(sds_map_t *header_map);

sds getContentLocation(sds_map_t *header_map);

sds getContentLength(sds_map_t *header_map);

sds getContentType(sds_map_t *header_map);

sds_map_t *extract_header(char *buffer);

bool isHTML(sds_map_t *header_map);

bool isBufferSufficient(sds_map_t *header_map);

bool parse_int(sds string, int *parse_result);

char *locate_body(char *buffer);

#endif //COMP30023_2020_PROJECT1_PARSER_H
