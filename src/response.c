//
// Created by Haswe on 3/23/2020.
//

#include "response.h"

#define HEADER_BODY_SEPARATOR_SIZE 4

Response *parse_response(sds *buffer) {
    sds message = *buffer;
    Response *response = (Response *) malloc(sizeof(*response));
    // separator between response header and body
    sds header_body_separator = "\r\n\r\n";

    char *where_body_is = strstr(message, header_body_separator);
    int header_size = strlen(message) - strlen(where_body_is);
    int body_size = strlen(message) - header_size - HEADER_BODY_SEPARATOR_SIZE;

    // Initialise a map to store name-value pair
    sds_map_t *map = malloc(sizeof(*map));
    map_init(map);
    response->header = map;

    // Copy header from response buffer
    sds header = sdscpylen(sdsempty(), message, header_size);

    // Copy body from response buffer
    response->body = sdscpylen(sdsempty(), where_body_is + HEADER_BODY_SEPARATOR_SIZE, body_size);

    sds *status_line, *split;
    int header_count, field_count, split_count;

    // Tokenize header into lines
    sds *lines = sdssplitlen(header, sdslen(header), "\r\n", 2, &header_count);

    for (int j = 0; j < header_count; j++) {
        if (j == 0) {
            // Split status line
            status_line = sdssplitlen(lines[j], sdslen(lines[j]), " ", 1, &field_count);
            response->version = sdsnew(status_line[0]);
            // convert status code to int
            response->status_code = atoi(status_line[1]);
            response->reason_phrase = sdsnew(status_line[2]);
            sdsfreesplitres(status_line, field_count);
        } else {
            split = sdssplitlen(lines[j], sdslen(lines[j]), ":", 1, &split_count);
            // Add header to map
            map_set(response->header, split[0], sdstrim(split[1], " \n"));
            sdsfreesplitres(split, split_count);
        }
    }
    sdsfreesplitres(lines, header_count);
    return response;
}

void print_header(Response *response) {
    sds key;
    map_iter_t iter = map_iter(response->header);
    printf("----- RESPONSE HEADER ------\n");
    while ((key = (char *) map_next(response->header, &iter))) {
        printf("%s -> %s\n", key, *map_get(response->header, key));
    }
}

void print_body(Response *response) {
    printf("----- RESPONSE BODY ------\n");
    printf("%s\n", response->body);
}

void free_response(Response *response) {
    sdsfree(response->version);
    response->version = NULL;
    sdsfree(response->reason_phrase);
    response->reason_phrase = NULL;
    sdsfree(response->body);
    response->body = NULL;
    map_deinit(response->header);
    response->header = NULL;
    free(response);
}

