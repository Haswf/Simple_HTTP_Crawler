//
// Created by Haswe on 3/25/2020.
//


#include "crawler.h"
#include "url.h"

int main(int agrc, char *argv[]) {

    if (agrc < 1) {
        printf("Usage: crawler [URL]");
        exit(0);
    }
    int total = 0;
    int failure = 0;
    log_set_level(LOG_LEVEL);

    int_map_t *seen = malloc(sizeof(*seen));
    map_init(seen);

    sds_vec_t *job_queue = malloc(sizeof(*job_queue));
    vec_init(job_queue);

//    sds initial = sdsnew();
    add_absolute_to_queue(argv[1], seen, job_queue);
//    vec_push(job_queue, );
//    sdsfree(initial);

    while (job_queue->length > 0) {
        int error = 0;
        sds url = vec_pop(job_queue);
        sds key = build_key(url);
        int *status = map_get(seen, key);
        sdsfree(key);
        // Fetch the page only if we've never visited it before or it has been mark as retry
        if (!status || *status == RETRY_FLAG) {
            error = do_crawler(url, job_queue, seen);
        }

        failure += error;
        total++;


    }
    map_deinit(seen);
    free(seen);
    vec_init(job_queue);
    free(job_queue);

    printf("Total Success: %d\nTotal Failure: %d\n", total - failure, failure);
}


int do_crawler(sds url, sds_vec_t *job_queue, int_map_t *seen) {
    int error = 0;
    if (is_valid_url(url)) {
        url_t *parse_result = parse_url(url);
        // If given url doesn't contain any path, concate / as default path
        if (!parse_result->path) {
            parse_result->path = sdsnew("/");
        }
        // Set path to / if none is given
        log_info("Fetching: %s", url);
        Request *request = create_http_request(sdsnew(parse_result->authority), sdsnew(parse_result->path),
                                               sdsnew("GET"), sdsnew(""));
        add_header(request, "Connection", CONNECTION);
        add_header(request, "User-Agent", USER_AGENT);
        add_header(request, "Accept", HTML_CONTENT_TYPE);

        Response *response = send_http_request(request, PORT, &error);

        if (!error) {
            validate_content_length(response);
            process_header(response);
            print_body(response);

            // Successful
            if (response->status_code / 100 == 2) {
                error = success_handler(parse_result, response, job_queue, seen);
            }
            // Redirection
            else if (response->status_code / 100 == 3) {
                error = redirection_handler(parse_result, response, job_queue, seen);
            }

                // Server error
            else if (response->status_code / 100 == 5) {
                if (response->status_code == SERVICE_UNAVAILABLE || response->status_code == GATEWAY_TIMEOUT) {
                    error = retry_handler(parse_result, response, job_queue, seen);
                } else {
                    error = failure_handler(parse_result, response, seen);
                }

            } else {
                error = failure_handler(parse_result, response, seen);
            }
            free_response(response);
        }
        free_url(parse_result);
        free_request(request);
    } else {
        log_error("Invalid URL - Missing scheme or host");
        error = 1;
    }
    return error;
}

int success_handler(url_t *url, Response *response, sds_vec_t *job_queue, int_map_t *seen) {
    if (content_type_validation(response->header)) {
        sds key = build_key(url->raw);
        mark_visited(key, seen);
        sdsfree(key);

        sds alternative_path = getContentLocation(response->header);
        if (alternative_path) {
            url_t *resolved = resolve_reference(alternative_path, url->raw);
            key = build_key(resolved->raw);
            mark_visited(resolved->raw, seen);
            log_debug("Alternative location %s saved", resolved->raw);
            sdsfree(key);
            free_url(resolved);
        }
        search_and_add_url(url, response->body, job_queue, seen);
        log_info("\t|- succeeded\t%d", response->status_code);
    }
    return 0;
}

int redirection_handler(url_t *url, Response *response, sds_vec_t *job_queue, int_map_t *seen) {
    sds key = build_key(url->raw);
    mark_visited(key, seen);
    sdsfree(key);

    sds redirect_to = getLocation(response->header);
    if (redirect_to != NULL && url_validation(url->raw, redirect_to)) {
        log_info("\t|- succeeded\t%d", response->status_code);
        log_info("\t|- Redirect to %s", redirect_to);
        // TODO: check if redirected url follows rules in spec
        add_absolute_to_queue(redirect_to, seen, job_queue);
    }
    return 0;
}

sds getLocation(sds_map_t *header_map) {
    sds *redirect_to = (sds *) map_get(header_map, LOCATION);
    if (redirect_to) {
        return *redirect_to;
    } else {
        return NULL;
    }
}


sds getContentLocation(sds_map_t *header_map) {
    sds *redirect_to = (sds *) map_get(header_map, CONTENT_LOCATION);
    if (redirect_to) {
        return *redirect_to;
    } else {
        return NULL;
    }
}

sds getContentLength(sds_map_t *header_map) {
    sds *content_length = (sds *) map_get(header_map, CONTENT_LENGTH);
    if (content_length) {
        return *content_length;
    } else {
        return NULL;
    }
}

sds getContentType(sds_map_t *header_map) {
    sds *type = (sds *) map_get(header_map, CONTENT_TYPE);
    if (type) {
        return *type;
    } else {
        return NULL;
    }
}


int failure_handler(url_t *url, Response *response, int_map_t *seen) {
    sds key = build_key(url->raw);
    mark_failure(key, seen);
    sdsfree(key);

    log_info("\t|- failed\t\t%d\tMarked as failure", response->status_code);
    return 1;
}

/**
 *
 * @param url
 * @param response
 * @param job_queue
 * @param seen
 * @return
 */
int retry_handler(url_t *url, Response *response, sds_vec_t *job_queue, int_map_t *seen) {
    sds key = build_key(url->raw);
    int *status = map_get(seen, key);
    sdsfree(key);

    // If a page has been fetched and failed
    if (status && *status == RETRY_FLAG) {
        log_info("\t|- Retry failed\t\t%d\t No further retry will be attempted",
                 response->status_code);
    } else {
        mark_retry(url->raw, seen, job_queue);
        log_info("\t|- failed\t\t%d\tRetry scheduled", response->status_code);
    }
    return 1;
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
    sds key = build_key(url);
    int result = map_set(seen, key, VISITED_FLAG);
    sdsfree(key);
    return result;
}

/**
 * Mark an url as failure. No further attempt will be made to fetch the page
 * @param url
 * @param seen
 * @return
 */
int mark_failure(sds url, int_map_t *seen) {
    sds key = build_key(url);
    int result = map_set(seen, key, FAILURE_FLAG);
    sdsfree(key);
    return result;
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

    sds key = build_key(abs_url);
    int *status = map_get(seen, key);
    sdsfree(key);
    if (status == NULL || *status == RETRY_FLAG) {
        log_trace("\t|+ %s added to the job queue", abs_url);
        return vec_push(job_queue, abs_url);
    }
    return -1;
}


int validate_content_length(Response *response) {
    sds content_length = getContentLength(response->header);
    if (content_length != NULL) {
        int actual = sdslen(response->body);
        int expected = (int) atoi(content_length);
        if (expected == actual) {
            return 0;
        } else {
            log_info("\t|- truncated page: Expected length: %d\t actual length:%d", expected, actual);
            return 1;
        }
    }
}

sds build_key(sds url) {
    url_t *result = parse_url(url);
    if (!result) {
        return sdscatprintf(sdsempty(), "%s://%s%s", result->scheme, result->authority, result->path);
    }
    sds key = sdscatprintf(sdsempty(), "%s://%s%s", result->scheme, result->authority, result->path);
    free_url(result);
    return key;
}

/**
 * Validate if a url is valid.
 * @param src
 * @param target
 * @return
 */
bool url_validation(sds src, sds target) {
    int src_count, target_count;
    url_t *src_parsed = parse_url(src);
    url_t *target_parsed = parse_url(target);

    if (!src_parsed || !target_parsed) {
        return false;
    }

    sds src_host = sdsnew(src_parsed->authority);
    sds target_host = sdsnew(target_parsed->authority);

    // TODO: Scheme validation is not working
//    if (!strncmp(target_parsed->scheme, src_parsed->scheme, strlen(src_parsed->scheme))) {
//        return false;
//    }

    free_url(src_parsed);
    free_url(target_parsed);

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

bool content_type_validation(sds_map_t *header_map) {
    sds type = getContentType(header_map);
    // if content type header is not presented
    if (type == NULL) {
        log_info("\t|- Content type validation failed: Content-Type is missing");
        return false;
    }
    // if text/html is found in content-type header
    if (strstr(type, HTML_CONTENT_TYPE)) {
        return true;
    }
    log_info("\t|- Content type validation failed: expected: %s actual: %s", HTML_CONTENT_TYPE, type);
    return false;
}
