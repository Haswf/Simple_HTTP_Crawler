//
// Created by Haswe on 3/25/2020.
//

#ifndef COMP30023_2020_PROJECT1_CRAWLER_H
#define COMP30023_2020_PROJECT1_CRAWLER_H

#include "HTTP.h"
#include "../lib/url_parser/url_parser.h"
#include "collection.h"
#include "parser.h"
#include "config.h"
#include <stdbool.h>

int process_header(Response *response);

int do_crawler(sds url, sds_vec_t *job_queue, int_map_t *seen);

int mark_visited(sds url, int_map_t *seen);

int mark_retry(sds url, int_map_t *seen, sds_vec_t *job_queue);

int mark_failure(sds url, int_map_t *seen);

sds relative_to_absolute(sds path, sds host);

int validate_content_length(Response *response);

int add_to_queue(sds abs_url, int_map_t *seen, sds_vec_t *job_queue);

int add_relative_to_queue(sds host, sds path, int_map_t *seen, sds_vec_t *job_queue);

int add_absolute_to_queue(sds abs_url, int_map_t *seen, sds_vec_t *job_queue);

int is_valid_url(sds url);

bool domain_validation(sds src, sds target);

sds add_scheme(sds url, sds header);

#endif //COMP30023_2020_PROJECT1_CRAWLER_H
