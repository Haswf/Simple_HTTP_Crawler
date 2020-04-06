/**
 * Module to handle HTTP response
 * Created by Shuyang Fan on 3/23/2020.
 */

#include "response.h"
#include "HTTP.h"

/**
 * Parse content in buffer into a response_t
 * @param buffer
 * @return
 */
response_t *parse_response(sds *buffer) {
    sds message = *buffer;
    response_t *response = (response_t *) malloc(sizeof(*response));
    // separator between response header and body
    char *header_body_separator = "\r\n\r\n";

    /*
     * Locate start position of body to determine body and header size
     */
    char *where_body_is = strstr(message, header_body_separator);
    int header_size = (int) strlen(message) - (int) strlen(where_body_is);
    int body_size = (int) strlen(message) - header_size - HEADER_BODY_SEPARATOR_SIZE;

    /* Initialise a map to store headers */
    response->header = malloc(sizeof(*response->header));
    map_init(response->header);

    // Copy header from response buffer
    sds header = sdscpylen(sdsempty(), message, header_size);

    // Copy body from response buffer
    response->body = sdscpylen(sdsempty(), where_body_is + HEADER_BODY_SEPARATOR_SIZE, body_size);

    int header_count, field_count;
    header_count = 0;
    field_count = 0;

    // Tokenize header into lines
    sds *lines = sdssplitlen(header, sdslen(header), "\r\n", 2, &header_count);

    for (int j = 0; j < header_count; j++) {
        // Split status line
        if (j == 0) {
            sds *status_line = sdssplitlen(lines[j], sdslen(lines[j]), " ", 1, &field_count);
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
//            sdsfree(value);
        }
    }
    /*
     * Clean up
     */
    sdsfreesplitres(lines, header_count);
    sdsfree(header);
    return response;
}

/**
 * Print headers of a response
 * @param response
 */
void print_header(response_t *response) {
    sds key;
    map_iter_t iter = map_iter(response->header);
    log_trace("----- RESPONSE HEADER ------");
    while ((key = (sds) map_next(response->header, &iter))) {
        log_trace("%s -> %s\n", key, *map_get(response->header, key));
    }
}

/**
 * Print body of a response
 * @param response
 */
void print_body(response_t *response) {
    log_trace("----- RESPONSE BODY ------");
    log_trace("%s", response->body);
}

/**
 * Free response
 * @param response
 */
void free_response(response_t *response) {
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

/**
 * Convert all characters in a string into lower case
 * @param string
 * @return
 */
sds lower(sds string) {
    for (int i = 0; i <= sdslen(string); i++) {
        string[i] = (char) tolower(string[i]);
    }
    return string;
}

