//
// Created by Haswe on 3/25/2020.
//


#include "crawler.h"
#include "url.h"

/**
 * Initialise a queue and a map for the crawler
 * I know the use of double pointer is a bit ugly...
 * @param seen
 * @param job_queue
 * @return
 */
int init(int_map_t **seen, sds_vec_t **job_queue) {
    log_set_level(LOG_LEVEL);
    *seen = malloc(sizeof(**seen));
    map_init(*seen);
    *job_queue = malloc(sizeof(**job_queue));
    vec_init(*job_queue);
    return *job_queue && *seen;
}

int deinit(int_map_t **seen, sds_vec_t **job_queue) {
    map_deinit(*seen);
    free(*seen);
    *seen = NULL;
    vec_init(*job_queue);
    free(*job_queue);
    *job_queue = NULL;
    return !(*job_queue) && !(*seen);
}

int main(int agrc, char *argv[]) {
    int_map_t *seen = NULL;
    sds_vec_t *job_queue = NULL;

    if (agrc < 1) {
        printf("Usage: crawler [URL]");
        exit(0);
    }
    int total = 0;
    int failure = 0;

    init(&seen, &job_queue);

    sds_map_t *common_header = malloc(sizeof(*common_header));
    map_init(common_header);

    map_set(common_header, "Connection", CONNECTION);
    map_set(common_header, "User-Agent", USER_AGENT);
    map_set(common_header, "Accept", HTML_CONTENT_TYPE);

    sds initial = sdsnew(argv[1]);
    add_to_queue(initial, seen, job_queue);

    while (job_queue->length > 0) {
        int error = 0;
        sds url = vec_pop(job_queue);

        sds key = build_key(url);
        int *status = map_get(seen, key);
        sdsfree(key);
        // Fetch the page only if we've never visited it before or it has been mark as retry
        if (!status || *status == RETRY_FLAG) {
            error = do_crawler(url, sdsnew("GET"), sdsempty(), job_queue, seen, common_header);
            total++;
        } else if (*status == POST_FLAG) {
            error = do_crawler(url, sdsnew("POST"), sdsempty(), job_queue, seen, common_header);
            total++;
        } else if (*status == AUTH_FLAG) {
            map_set(common_header, AUTHORIZATION, sdscatprintf(sdsempty(), "Basic %s", TOKEN));
            error = do_crawler(url, sdsnew("GET"), sdsempty(), job_queue, seen, common_header);
            map_remove(common_header, AUTHORIZATION);
            total++;
        }
        failure += error;
    }
    deinit(&seen, &job_queue);

    sdsfree(initial);

    log_info("Total Success: %d\nTotal Failure: %d\n", total - failure, failure);
}


int do_crawler(sds url, sds method, sds body, sds_vec_t *job_queue, int_map_t *seen, sds_map_t *header) {
    url_t *parse_result = NULL;
    Request *request = NULL;
    response_t *response = NULL;
    int error = 0;
    if (!is_valid_url(url)) {
        log_error("Invalid URL %s- Missing scheme or host", url);
        return clean_up(request, response, parse_result);
    }

    parse_result = parse_url(url);
    // If given url doesn't contain any path, concate / as default path
    if (!parse_result->path) {
        parse_result->path = sdsnew("/");
        parse_result->raw = recomposition(parse_result);
        url = parse_result->raw;
    }
    // Set path to / if none is given
    log_info("Fetching: %s", url);
    printf("%s\n", url);

    request = create_http_request(sdsnew(parse_result->authority), sdsnew(parse_result->path),
                                  method, body);
    set_headers(request, header);

    response = send_http_request(request, PORT, &error);
    if (error) {
        failure_handler(parse_result, response, seen);
        return clean_up(request, response, parse_result);
    }
    // print response header and body for debugging
    print_header(response);
    print_body(response);

    if (is_truncated_page(response)) {
        error = retry_handler(parse_result, response, job_queue, seen);
        mark_retry(sdsnew(parse_result->raw), seen);
        return clean_up(request, response, parse_result);
    } else {
        error = response_to_http_status(response, parse_result, job_queue, seen);
    }

    clean_up(request, response, parse_result);
    return error;
}

void set_headers(Request *request, sds_map_t *header_map) {
    sds key;
    map_iter_t iter = map_iter(header_map);

    while ((key = (sds) map_next(header_map, &iter))) {
        add_header(request, key, *map_get(header_map, key));
    }
}

int clean_up(Request *request, response_t *response, url_t *parse_result) {
    if (request) {
        free_request(request);
    }
    if (response) {
        free_response(response);
    }
    if (parse_result) {
        free_url(parse_result);
    }
    return 1;
}

int response_to_http_status(response_t *response, url_t *parse_result, sds_vec_t *job_queue, int_map_t *seen) {
    int error = 0;

    // Success
    if (response->status_code / 100 == 2) {
        search_and_add_url(parse_result, response->body, job_queue, seen);
        error = success_handler(parse_result, response, seen);
    }
        // Redirection
    else if (response->status_code / 100 == 3) {
        error = redirection_handler(parse_result, response, job_queue, seen);
    }

        // Client Error
    else if (response->status_code / 100 == 4) {
        if (response->status_code == NOT_FOUND || response->status_code == GONE ||
            response->status_code == URI_TOO_LONG) {
            search_and_add_url(parse_result, response->body, job_queue, seen);
            error = failure_handler(parse_result, response, seen);
//        } else if (response->status_code == URI_TOO_LONG) {
//            search_and_add_url(parse_result, response->body, job_queue, seen);
////            mark_post(parse_result, response, seen);
////            vec_push(job_queue, sdsnew(parse_result->raw));
        } else if (response->status_code == UNAUTHORISED) {
            error = retry_handler(parse_result, response, job_queue, seen);
            mark_auth_required(parse_result->raw, seen);
        } else {
            error = failure_handler(parse_result, response, seen);
        }
    }

        // Server error
    else if (response->status_code / 100 == 5) {
        if (response->status_code == SERVICE_UNAVAILABLE || response->status_code == GATEWAY_TIMEOUT) {
            // TODO: The crawler will attempt to revisit a page when the status code of a response indicates a temporary failure.
            error = retry_handler(parse_result, response, job_queue, seen);
            mark_retry(parse_result->raw, seen);
            search_and_add_url(parse_result, response->body, job_queue, seen);
        } else {
            error = failure_handler(parse_result, response, seen);
        }

    }
        /*
         * TODO: Note that responses with other status codes may be thrown by the server (e.g., when a malformed request is made) but need not be parsed.
         */
    else {
        error = failure_handler(parse_result, response, seen);
    }
    return error;
}

int success_handler(url_t *url, response_t *response, int_map_t *seen) {
    if (content_type_validation(response->header)) {
        mark_visited(url->raw, seen);

        // TODO: "Two pages are considered to be the same page if the URLs indicate that they are the same."
        sds alternative_path = getContentLocation(response->header);
        if (alternative_path) {
            url_t *resolved = resolve_reference(alternative_path, url->raw);
            mark_visited(resolved->raw, seen);
            log_debug("Alternative location %s saved", resolved->raw);
            free_url(resolved);
        }
        log_info("\t|- succeeded\t%d", response->status_code);
    }
    return 0;
}

int redirection_handler(url_t *url, response_t *response, sds_vec_t *job_queue, int_map_t *seen) {
    sds key = build_key(url->raw);
    mark_visited(url->raw, seen);
    sdsfree(key);
    log_info("\t|- succeeded\t%d", response->status_code);

    sds redirect_to = getLocation(response->header);
    if (redirect_to != NULL && url_validation(url->raw, redirect_to)) {
        log_info("\t|- Redirect to %s", redirect_to);
        // TODO: check if redirected url follows rules in spec
        add_to_queue(redirect_to, seen, job_queue);
    } else {
        log_info("\t|- Redirection URL is not supported %s", redirect_to);
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


int failure_handler(url_t *url, response_t *response, int_map_t *seen) {
    mark_failure(url->raw, seen);
    if (response) {
        log_info("\t|- failed\t\t%d\tMarked as failure", response->status_code);
    } else {
        log_info("\t|- failed\t No response received");
    }
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
int retry_handler(url_t *url, response_t *response, sds_vec_t *job_queue, int_map_t *seen) {
    sds key = build_key(url->raw);
    int *status = map_get(seen, key);
    sdsfree(key);

    // If a page has been fetched and failed
    if (status && (*status == RETRY_FLAG || *status == AUTH_FLAG)) {
        log_info("\t|- Retry failed\t\t%d\t No further retry will be attempted",
                 response->status_code);
    } else {
        vec_push(job_queue, sdsdup(url->raw));
        log_info("\t|- failed\t\t%d\tRetry scheduled", response->status_code);
    }
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
int authorization_handler(url_t *url, response_t *response, sds_vec_t *job_queue, int_map_t *seen) {
    sds key = build_key(url->raw);
    int *status = map_get(seen, key);
    sdsfree(key);

    // If a page has been fetched and failed
    if (status && *status == UNAUTHORISED) {
        log_info("\t|- Retry failed\t\t%d\t No further retry will be attempted",
                 response->status_code);
    } else {
        mark_retry(sdsnew(url->raw), seen);
        vec_push(job_queue, sdsdup(url->raw));
        log_info("\t|- failed\t\t%d\tRetry scheduled", response->status_code);
    }
    return 1;
}

int seen_set(int_map_t *seen, sds key, int value) {
    sds _key = build_key(key);
    int result = map_set(seen, _key, value);
    sdsfree(_key);
    return result;
}

/**
 * Mark an url as visited. There will add its visit count by 1.
 * @param url
 * @param seen
 * @return
 */
int mark_visited(sds url, int_map_t *seen) {
    return seen_set(seen, url, VISITED_FLAG);;
}

/**
 * Mark an url as failure. No further attempt will be made to fetch the page
 * @param url
 * @param seen
 * @return
 */
int mark_failure(sds url, int_map_t *seen) {
    return seen_set(seen, url, FAILURE_FLAG);;
}


int mark_post(sds url, int_map_t *seen) {
    return seen_set(seen, url, POST_FLAG);;
}

/**
 * Mark an url to retry later
 * @param url
 * @param seen
 * @param job_queue
 * @return
 */
int mark_retry(sds url, int_map_t *seen) {
    return seen_set(seen, url, RETRY_FLAG);;
}

/**
 * Mark an url to retry later
 * @param url
 * @param seen
 * @param job_queue
 * @return
 */
int mark_auth_required(sds url, int_map_t *seen) {
    return seen_set(seen, url, AUTH_FLAG);;
}

/**
 * Add an absolute url to job queue if it has't been fetched before or has been marked for retry
 * @param abs_url
 * @param seen
 * @param job_queue
 * @return
 */
int add_to_queue(sds abs_url, int_map_t *seen, sds_vec_t *job_queue) {
    if (abs_url == NULL || seen == NULL || job_queue == NULL) {
        return 1;
    }
    if (!sdslen(abs_url)) {
        return 1;
    }

    sds key = build_key(abs_url);
    int *status = map_get(seen, key);
    sdsfree(key);

    sdstrim(abs_url, " \n");
    if (status == NULL || *status == RETRY_FLAG) {
        log_trace("\t|+ %s added to the job queue", abs_url);
        return vec_push(job_queue, abs_url);
    }
    return -1;
}


bool is_truncated_page(response_t *response) {
    sds content_length = getContentLength(response->header);
    if (content_length != NULL) {
        int actual = sdslen(response->body);
        int expected = (int) atoi(content_length);
        if (expected == actual) {
            return false;
        } else {
            log_info("\t|- truncated page: Expected length: %d\t actual length:%d", expected, actual);
            return true;
        }
    }
    // Any response with no content-length header with be discarded
    return true;
}

/**
 * Converts a full url to a representative string as key for the "seen" map.
 * Only parts of the url, namely scheme, authority and path will be used to
 * construct the key.
 * @param url
 * @return
 */
sds build_key(sds url) {
    url_t *result = parse_url(url);
    sds key = sdsempty();
    if (result->scheme) {
        key = sdscatprintf(key, "%s://", sdsnew(lower(result->scheme)));
    }
    if (result->authority) {
        key = sdscat(key, sdsnew(lower(result->authority)));
    }
    if (result->path) {
        key = sdscat(key, sdsnew(result->path));
    } else {
        key = sdscatfmt(key, "/");
    }
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

    if (!src || !target) {
        return false;
    }

    int src_count, target_count;
    url_t *src_parsed = parse_url(src);
    url_t *target_parsed = parse_url(target);

    if (!is_valid_url(src) || !is_valid_url(target)) {
        return false;
    }

    sds src_host = sdsnew(src_parsed->authority);
    sds target_host = sdsnew(target_parsed->authority);

//    // TODO: Only parses pages that use http
//    if (compare_scheme(src_parsed, target_parsed)) {
//        log_trace("Scheme Validation failed: %s %s", src_parsed->scheme, target_parsed->scheme);
//        return false;
//    }

    free_url(src_parsed);
    free_url(target_parsed);

    sdstrim(src_host, " \n\r");
    sdstrim(target_host, " \n\r");

    sds *src_token = sdssplitlen(src_host, sdslen(src_host), ".", 1, &src_count);
    sds *target_token = sdssplitlen(target_host, sdslen(target_host), ".", 1, &target_count);

    if (src_count != target_count) {
        log_trace("Host Validation failed: %s %s", src, target);
        return false;
    }

    for (int index = 1; index < src_count; index++) {
        if (strcmp(src_token[index], target_token[index]) != 0) {
            log_trace("Host Validation failed: %s %s: %s %s mismatch at index %d", src, target, src_token[index],
                      target_token[index], index);
            return false;
        }
    }
    sdsfreesplitres(src_token, src_count);
    sdsfreesplitres(target_token, target_count);
    return true;
}

bool content_type_validation(sds_map_t *header_map) {
    // Retrieve content type from map
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

/**
 * Compares the schemes of two urls
 * @param url1
 * @param url2
 * @return
 */
int compare_scheme(url_t *url1, url_t *url2) {
    if (!url1 || !url2) {
        return 1;
    }
    if (url1->scheme && url2->scheme) {
        return sdscmp(url1->scheme, url2->scheme);
    }
    return 1;
}
