//
// Created by Haswe on 3/25/2020.
//
#include "config.h"
#include "crawler.h"

int main(int agrc, char *argv[]) {
    int total = 0;
    int failure = 0;
    log_set_level(LOG_LEVEL);

    int_map_t *seen = malloc(sizeof(*seen));
    map_init(seen);

    sds_vec_t *job_queue = malloc(sizeof(*job_queue));
    vec_init(job_queue);

    vec_insert(job_queue, 0, argv[1]);

    while (job_queue->length > 0) {
        int error = 0;
        sds url = vec_pop(job_queue);
        int *count = map_get(seen, url);
        if (count == NULL || *count == RETRY_FLAG) {
            error = do_crawler(url, job_queue, seen);
            failure += error;
            total++;
        } else if (*count == FAILURE_FLAG) {
            log_info("Skipping %s\tMax Retry reached", url, MAX_RETRY);
        }
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
        Response *response = send_http_request(request, PORT, &error);

//        process_header(response);
//        print_body(response);

        if (!error) {
            // Successful
            if (response->status_code / 100 == 2) {
                add_url(request, response->body, job_queue, seen);
                mark_visited(url, seen);
                log_info("Fetching %s succeeded\t%d", url, response->status_code);
            }
                // Redirection
            else if (response->status_code / 100 == 3) {
                mark_visited(url, seen);
                sds *redirect_to = (sds *) map_get(response->header, "Location");
                if (redirect_to != NULL) {
                    log_info("Fetching %s succeeded\tRedirect to %s", url, *redirect_to);
                    vec_push(job_queue, *redirect_to);
                }
                error = 1;
            }

                // Client Error
            else if (response->status_code / 100 == 4) {
                mark_visited(url, seen);
                log_info("Fetching %s failed\tMarked as failure", url, response->status_code);
                error = 1;
            }
                // Server error
            else if (response->status_code / 100 == 5) {
                if (response->status_code == SERVICE_UNAVAILABLE) {
                    mark_retry(url, seen, job_queue);
                    log_info("Fetching %s failed\t%d\tRetry scheduled", url, response->status_code);
                    error = 1;
                } else if (response->status_code == GATEWAY_TIMEOUT) {
                    mark_retry(url, seen, job_queue);
                    log_info("Fetching %s failed\t%d\tRetry scheduled", url, response->status_code);
                    error = 1;
                } else {
                    mark_failure(url, seen);
                    log_info("Fetching %s failed\t%d\tMarked as failure", url, response->status_code);
                    error = 1;
                }
            } else {
                mark_failure(url, seen);
                error = 1;
            }
            free_response(response);
        }
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

int add_to_queue(sds host, sds url, int_map_t *seen, sds_vec_t *job_queue) {
    if (parse_url(url) == NULL) {
        sds abs = relative_to_absolute(url, host);
        if (abs) {
            log_debug("Relative to absolute %s -> %s", url, abs);
        }
        url = abs;
    }

    if (url) {
        int *count = map_get(seen, url);
        if (count == NULL || *count == RETRY_FLAG) {
            log_debug("%s added to the job queue", url);
            return vec_push(job_queue, url);
        }
        return 1;
    }
}

sds relative_to_absolute(sds path, sds host) {
    sds abs;
    abs = sdscatprintf(sdsempty(), "http://%s/%s", host, path);
    if (parse_url(abs)) {
        return abs;
    }
    return NULL;
}