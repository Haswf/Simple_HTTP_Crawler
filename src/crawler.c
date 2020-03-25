//
// Created by Haswe on 3/25/2020.
//

#include "crawler.h"
#define PORT 80

int main(int agrc, char *argv[]) {
    int success = 0;
    int failure = 0;
    int_map_t *seen = malloc(sizeof(*seen));
    map_init(seen);

    sds_vec_t *job_queue = malloc(sizeof(*job_queue));
    vec_init(job_queue);

    vec_insert(job_queue, 0, argv[1]);

    while (job_queue->length > 0) {
        int status = 0;
        sds url = vec_pop(job_queue);
        int *count = map_get(seen, url);
        if (count == NULL) {
            log_info("Fetching %s", url);
//            printf("Fetching: %s\n", url);
            status = do_crawler(url, job_queue, seen);
            map_set(seen, url, 1);
        } else if (*count > 0) {
            log_info("Skipping %s", url);
//            printf("Skipping: %s\n", url);
        } else {
//            printf("Fetching: %s\n", url);
            log_info("Fetching %s", url);
            status = do_crawler(url, job_queue, seen);
            map_set(seen, url, *count + 1);
        }
        if (status != 0) {
            log_error("Aborting %s", url);
            failure++;
        } else {
            success++;
        }
    }
    printf("Total Success: %d\nTotal Failure: %d\n", success, failure);
}


int do_crawler(sds url, sds_vec_t *job_queue, int_map_t *seen) {
    int error = 0;
    parsed_url_t *parse_result = parse_url(url);
    if (parse_result != NULL) {
        // Set path to / if none is given
        sds path = parse_result->path == NULL ? sdsnew("/") : sdscatfmt(sdsempty(), "/%s", parse_result->path);
        Request *request = create_http_request(sdsnew(parse_result->host), path, sdsnew("GET"), sdsnew(""));

        add_header(request, "Connection", "close");
        add_header(request, "User-Agent", "shuyangf");
        Response *response = send_http_request(request, PORT, &error);

//        process_header(response);
//        print_body(response);
        map_set(seen, url, 1);
        if (!error) {
            add_url(response, job_queue, seen);
            free_response(response);
        }
        free_request(request);
        return error;
    }
}

int process_header(Response *response) {
    print_header(response);
}
