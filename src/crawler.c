//
// Created by Haswe on 3/25/2020.
//

#include "crawler.h"
#define PORT 80

int main(int agrc, char *argv[]) {
    int total = 0;
    int_map_t *seen = malloc(sizeof(*seen));
    map_init(seen);

    sds_vec_t *job_queue = malloc(sizeof(*job_queue));
    vec_init(job_queue);

    vec_insert(job_queue, 0, argv[1]);

    while (job_queue->length > 0) {
        sds url = vec_pop(job_queue);
        int *count = map_get(seen, url);
        if (count == NULL) {
            printf("Fetching: %s\n", url);
            do_crawler(url, job_queue, seen);
            map_set(seen, url, 1);
            total++;
        } else if (*count > 0) {
            printf("Skipping: %s\n", url);
        } else {
            printf("Fetching: %s\n", url);
            do_crawler(url, job_queue, seen);
            map_set(seen, url, *count + 1);
            total++;
        }
    }
    printf("Total: %d\n", total);
}


int do_crawler(sds url, sds_vec_t *job_queue, int_map_t *seen) {
    parsed_url_t *parse_result = parse_url(url);
    if (parse_result != NULL) {
        // Set path to / if none is given
        sds path = parse_result->path == NULL ? sdsnew("/") : sdscatfmt(sdsempty(), "/%s", parse_result->path);
        Request *request = create_http_request(sdsnew(parse_result->host), path, sdsnew("GET"), sdsnew(""));

        add_header(request, "Connection", "close");
        add_header(request, "User-Agent", "shuyangf");

        Response *response = send_http_request(request, PORT);
        free_request(request);
        request = NULL;

//    process_header(response);
//    print_body(response);
        map_set(seen, url, 1);
        add_url(response, job_queue, seen);
        free_response(response);
        response = NULL;
    }
}

int process_header(Response *response) {
    print_header(response);
}
