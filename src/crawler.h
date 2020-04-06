//
// Created by Haswe on 3/25/2020.
//

#ifndef COMP30023_2020_PROJECT1_CRAWLER_H
#define COMP30023_2020_PROJECT1_CRAWLER_H

#include <stdbool.h>

#include "request.h"
#include "response.h"
#include "HTTP.h"
#include "collection.h"
#include "parser.h"
#include "config.h"

int do_crawler(sds url, sds method, sds body, sds_vec_t *job_queue, int_map_t *seen, sds_map_t *header);

int mark_visited(sds url, int_map_t *seen);

int mark_retry(sds url, int_map_t *seen);

int mark_failure(sds url, int_map_t *seen);

bool is_truncated_page(response_t *response);

int add_to_queue(sds abs_url, int_map_t *seen, sds_vec_t *job_queue);

bool is_valid_url(sds url);

bool url_validation(sds src, sds target);

bool content_type_validation(sds_map_t *header_map);

int success_handler(url_t *url, response_t *response, int_map_t *seen);

int failure_handler(url_t *url, response_t *response, int_map_t *seen);

int redirection_handler(url_t *url, response_t *response, sds_vec_t *job_queue, int_map_t *seen);

int retry_handler(url_t *url, response_t *response, sds_vec_t *job_queue, int_map_t *seen);

sds build_key(sds url);

int response_to_http_status(response_t *response, url_t *parse_result, sds_vec_t *job_queue, int_map_t *seen);

int clean_up(request_t *request, response_t *response, url_t *parse_result);

void set_headers(request_t *request, sds_map_t *header_map);

int deinit(int_map_t **seen, sds_vec_t **job_queue);

int mark_post(sds url, int_map_t *seen);

int mark_auth_required(sds url, int_map_t *seen);

#endif //COMP30023_2020_PROJECT1_CRAWLER_H
