//
// Created by Haswe on 3/25/2020.
//


#include "crawler.h"
int main(int agrc, char *argv[]) {
    int total = 0;
    int failure = 0;
    log_set_level(LOG_LEVEL);

    int_map_t *seen = malloc(sizeof(*seen));
    map_init(seen);

    sds_vec_t *job_queue = malloc(sizeof(*job_queue));
    vec_init(job_queue);

    sds initial = sdsnew(argv[1]);

    if (!is_valid_url(initial)) {
        initial = add_scheme(initial, "http");
    }

    add_absolute_to_queue(initial, seen, job_queue);

    while (job_queue->length > 0) {
        int error = 0;
        sds url = vec_pop(job_queue);
        int *count = map_get(seen, url);
        // Fetch the page only if we've never visited it before or it has been mark as retry
        if (!count || *count == RETRY_FLAG) {
            error = do_crawler(url, job_queue, seen);
        }
//        else {
//            log_info("Skipping %-40s\t", url);
//        }
        failure += error;
        total++;

        // Skip page that has been marked as failure

    }
    printf("Total Success: %d\nTotal Failure: %d\n", total - failure, failure);
}


int do_crawler(sds url, sds_vec_t *job_queue, int_map_t *seen) {
    int error = 0;
    parsed_url_t *parse_result = parse_url(url);
    if (parse_result != NULL) {
        // Set path to / if none is given
        sds path = parse_result->path == NULL ? sdsnew("/") : sdscatfmt(sdsempty(), "/%s", parse_result->path);
        Request *request = create_http_request(sdsnew(parse_result->host), path, sdsnew("GET"), sdsnew(""));

        add_header(request, "Connection", CONNECTION);
        add_header(request, "User-Agent", USER_AGENT);
        add_header(request, "Accept", HTML_CONTENT_TYPE);

        Response *response = send_http_request(request, PORT, &error);
        int *count = map_get(seen, url);

        if (!error) {
            validate_content_length(response);
            process_header(response);
            print_body(response);

            // Successful
            if (response->status_code / 100 == 2 && content_type_validation(response)) {
                search_and_add_url(parse_result, response->body, job_queue, seen);
                mark_visited(url, seen);
                log_info("Fetching %-100s succeeded\t%d", url, response->status_code);
            }
                // Redirection
            else if (response->status_code / 100 == 3) {
                mark_visited(url, seen);
                sds *redirect_to = (sds *) map_get(response->header, LOCATION);
                if (redirect_to != NULL) {
                    log_info("Fetching %-100s succeeded\t%d", url, response->status_code);
                    log_info("\t|- Redirect to %s", *redirect_to);
                    add_absolute_to_queue(*redirect_to, seen, job_queue);
                }
                error = 1;
            }

                // Client Error
            else if (response->status_code / 100 == 4) {
                mark_visited(url, seen);
                log_info("Fetching %-100s failed\t\t%d", url, response->status_code);
                error = 1;
            }

                // Server error
            else if (response->status_code / 100 == 5) {
                if (response->status_code == SERVICE_UNAVAILABLE) {
                    if (*count == RETRY_FLAG) {
                        log_info("Fetching %-100s failed\t\t%d\t No retry will be attempted", url,
                                 response->status_code);
                    } else {
                        mark_retry(url, seen, job_queue);
                        log_info("Fetching %-100s failed\t\t%d\tRetry scheduled", url, response->status_code);
                    }
                } else if (response->status_code == GATEWAY_TIMEOUT) {
                    mark_retry(url, seen, job_queue);
                    log_info("Fetching %-100s failed\t\t%d\tRetry scheduled", url, response->status_code);
                    error = 1;
                } else {
                    mark_failure(url, seen);
                    log_info("Fetching %-100s failed\t\t%d\tMarked as failure", url, response->status_code);
                    error = 1;
                }
            } else {
                mark_failure(url, seen);
                log_info("Fetching %-100s failed\t\t%d\tMarked as failure", url, response->status_code);
                error = 1;
            }
            free_response(response);
        }
        parsed_url_free(parse_result);
        free_request(request);
        return error;
    }
}

int process_header(Response *response) {
    print_header(response);
}

/**
 * Mark an url as visited. There will add its visit count by 1.
 * @param url
 * @param seen
 * @return
 */
int mark_visited(sds url, int_map_t *seen) {
    int *count = map_get(seen, url);
    return map_set(seen, url, VISITED_FLAG);
}

/**
 * Mark an url as failure. No further attempt will be made to fetch the page
 * @param url
 * @param seen
 * @return
 */
int mark_failure(sds url, int_map_t *seen) {
    return map_set(seen, url, FAILURE_FLAG);
}

/**
 * Mark an url to retry later
 * @param url
 * @param seen
 * @param job_queue
 * @return
 */
int mark_retry(sds url, int_map_t *seen, sds_vec_t *job_queue) {
    map_set(seen, url, RETRY_FLAG);
    vec_push(job_queue, url);
    return 0;
}

/**
 * Concate path and url then add it to job queue
 * @param host
 * @param path
 * @param seen
 * @param job_queue
 * @return
 */
int add_relative_to_queue(sds host, sds path, int_map_t *seen, sds_vec_t *job_queue) {
    sds valid_url = resolve_referencing(path, host);
    if (valid_url) {
        log_debug("\t|+ Relative to absolute %s -> %s", path, valid_url);
        return add_to_queue(valid_url, seen, job_queue);
    }
}

int add_absolute_to_queue(sds abs_url, int_map_t *seen, sds_vec_t *job_queue) {
    if (abs_url == NULL) {
        return 1;
    }
    return add_to_queue(abs_url, seen, job_queue);
}

/**
 * Add an absolute url to job queue if it has't been fetched before or has been marked for retry
 * @param abs_url
 * @param seen
 * @param job_queue
 * @return
 */
int add_to_queue(sds abs_url, int_map_t *seen, sds_vec_t *job_queue) {
    int *count = map_get(seen, abs_url);
    if (count == NULL || *count == RETRY_FLAG) {
        log_debug("\t|+ %s added to the job queue", abs_url);
        return vec_push(job_queue, abs_url);
    }
    return -1;
}

/**
 * Check if a url looks valid by attempting to parse it
 * @param url
 * @return 0 for doesn't look an url, 1 for looks like an url
 */
bool is_valid_url(sds url) {
    parsed_url_t *result = parse_url(url);
    if (result == NULL) {
        return false;
    } else {
        free(result);
        return true;
    }
}

int validate_content_length(Response *response) {
    sds *content_length = (sds *) map_get(response->header, CONTENT_LENGTH);
    if (content_length != NULL) {
        int actual = sdslen(response->body);
        int expected = atoi(*content_length);
        if (expected == actual) {
            return 0;
        } else {
            log_warn("\t|- truncated page: Expected length: %d\t actual length:%d", expected, actual);
            return 1;
        }
    }
}

bool url_validation(sds src, sds target) {
    int src_count, target_count;
    parsed_url_t *src_parsed = parse_url(src);
    parsed_url_t *target_parsed = parse_url(target);

    if (!src_parsed || !target_parsed) {
        return false;
    }

    sds src_host = sdsnew(src_parsed->host);
    sds target_host = sdsnew(target_parsed->host);

    if (!strcmp(target_parsed->scheme, src_parsed->scheme)) {
        return false;
    }

    parsed_url_free(src_parsed);
    parsed_url_free(target_parsed);

    sds *src_token = sdssplitlen(sdstrim(src_host, " \n\r"), sdslen(src_host), ".", 1, &src_count);
    sds *target_token = sdssplitlen(sdstrim(target_host, " \n\r"), sdslen(target_host), ".", 1, &target_count);
    if (src_count != target_count) {
        log_trace("Domain Validation failed: %s %s", src, target);
        return false;
    }
    int index;
    for (index = 1; index < src_count; index++) {
        if (strcmp(src_token[index], target_token[index]) != 0) {
            log_trace("Domain Validation failed: %s %s: %s %s mismatch at index %d", src, target, src_token[index],
                      target_token[index], index);
            return false;
        }
    }
    sdsfreesplitres(src_token, src_count);
    sdsfreesplitres(target_token, target_count);
    return true;
}

sds add_scheme(sds url, sds header) {
    return sdscatprintf("%s://%s", url, header);
}

bool content_type_validation(Response *response) {
    sds *type = (sds *) map_get(response->header, CONTENT_TYPE);
    // if content type header is not presented
    if (type == NULL) {
        log_info("\t|- Content type validation failed: Content-Type is missing");
        return false;
    }
    // if text/html is found in content-type header
    if (strstr(*type, HTML_CONTENT_TYPE)) {
        return true;
    }
    log_info("\t|- Content type validation failed: expected: %s actual: %s", HTML_CONTENT_TYPE, *type);
    return false;
}
