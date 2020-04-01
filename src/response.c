//
// Created by Haswe on 3/23/2020.
//

#include "response.h"
#include "config.h"
#include "HTTP.h"


Response *parse_response(sds *buffer) {
    sds message = *buffer;
    Response *response = (Response *) malloc(sizeof(*response));
    // separator between response header and body
    char *header_body_separator = "\r\n\r\n";

    char *where_body_is = strstr(message, header_body_separator);
    int header_size = (int) strlen(message) - (int) strlen(where_body_is);
    int body_size = (int) strlen(message) - header_size - HEADER_BODY_SEPARATOR_SIZE;

    // Initialise a map to store name-value pair
    sds_map_t *map = malloc(sizeof(*map));
    map_init(map);
    response->header = map;

    // Copy header from response buffer
    sds header = sdscpylen(sdsempty(), message, header_size);

    // Copy body from response buffer
    response->body = sdscpylen(sdsempty(), where_body_is + HEADER_BODY_SEPARATOR_SIZE, body_size);

    sds *status_line;
    int header_count, field_count;

    // Tokenize header into lines
    sds *lines = sdssplitlen(header, sdslen(header), "\r\n", 2, &header_count);

    for (int j = 0; j < header_count; j++) {
        if (j == 0) {
            // Split status line
            status_line = sdssplitlen(lines[j], sdslen(lines[j]), " ", 1, &field_count);
            response->version = sdsnew(status_line[0]);
            // convert status code to int
            parse_int(status_line[1], &(response->status_code));
            response->reason_phrase = sdsnew(status_line[2]);
            sdsfreesplitres(status_line, field_count);
        } else {
            // Locate the first colon
            char *first_colon = strstr(lines[j], ":");
            sds name = sdscpylen(sdsempty(), lines[j], strlen(lines[j]) - strlen(first_colon));
            sds value = sdsnew(first_colon + 1);
            value = sdstrim(value, " \n");

            // Add header to map
            map_set(response->header, lower(name), value);
            sdsfree(name);
            sdsfree(value);
        }
    }
    sdsfreesplitres(lines, header_count);
    sdsfree(header);
    return response;
}

void print_header(Response *response) {
    sds key;
    map_iter_t iter = map_iter(response->header);
    log_trace("----- RESPONSE HEADER ------");
    while ((key = (sds) map_next(response->header, &iter))) {
        log_trace("%s -> %s\n", key, *map_get(response->header, key));
    }
}

void print_body(Response *response) {
    log_trace("----- RESPONSE BODY ------");
    log_trace("%s", response->body);
}

void free_response(Response *response) {
    sdsfree(response->version);
    response->version = NULL;
    sdsfree(response->reason_phrase);
    response->reason_phrase = NULL;
    sdsfree(response->body);
    response->body = NULL;
    map_deinit(response->header);
    free(response->header);
    response->header = NULL;
    free(response);
}

sds lower(sds string) {
//    printf("Before: %s\n", string);
    for (int i = 0; i <= sdslen(string); i++) {
        string[i] = (char) tolower(string[i]);
    }
//    printf("After: %s\n", string);
    return string;
}

