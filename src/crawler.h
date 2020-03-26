//
// Created by Haswe on 3/25/2020.
//

#ifndef COMP30023_2020_PROJECT1_CRAWLER_H
#define COMP30023_2020_PROJECT1_CRAWLER_H

#include "HTTP.h"
#include "../lib/url_parser/url_parser.h"
#include "collection.h"
#include "parser.h"

int process_header(Response *response);

int do_crawler(sds url, sds_vec_t *job_queue, int_map_t *seen);

int mark_visited(sds url, int_map_t *seen);

int mark_retry(sds url, int_map_t *seen, sds_vec_t *job_queue);

int mark_failure(sds url, int_map_t *seen);

int add_to_queue(sds host, sds url, int_map_t *seen, sds_vec_t *job_queue);

sds relative_to_absolute(sds path, sds host);

#endif //COMP30023_2020_PROJECT1_CRAWLER_H
