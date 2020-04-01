//
// Created by Haswell on 3/20/2020.
//

#include <stdbool.h>
#include "connection.h"
#include "request.h"
#include "response.h"
#include "config.h"
#include "crawler.h"
#include <errno.h>

Response *send_http_request(Request *request, int portno, int *error) {
    // Socket file descriptor
    int sockfd = 0;
    Response *response = NULL;
    // Create a connection
    *error = create_connection(request->host, portno, &sockfd);
    // Return NULL response if failed to create connection
    if (*error != 0) {
        return NULL;
    }
    // Convert request to a string
    sds reqString = HTTPRequestToString(request);
    log_trace("\n---- HTTP Request -----\n%s--------------------------", reqString);
    *error = send_to_server(sockfd, reqString);
    sdsfree(reqString);
    if (*error != 0) {
        return NULL;
    }
    char *buffer = (char *) calloc(RESPONSE_BUFFER, sizeof(buffer));
//    sds buffer = sdsnewlen("", RESPONSE_BUFFER);
    *error = receive_from_server(sockfd, &buffer);
    // free response buffer if something goes wrong
    if (*error != 0) {
        free(buffer);
        return NULL;
    } else {
        response = parse_response(&buffer);
        free(buffer);
    }

    *error = close_connection(sockfd);
    if (*error != 0) {
        free_response(response);
        return NULL;
    }

    return response;
}

/**
 * Check if the full header has been received
 * Return NULL if not yet received
 * @param buffer
 * @return
 */
char *locate_header(char *buffer) {
    char *header_body_separator = "\r\n\r\n";
    return strstr(buffer, header_body_separator);
}

sds_map_t *extract_header(char *buffer) {
    char *where_body_is = locate_header(buffer);
    if (!where_body_is) {
        return NULL;
    }
    int header_size = (int) strlen(buffer) - (int) strlen(where_body_is);
    sds header = sdscpylen(sdsempty(), buffer, header_size);
    sds *status_line;
    int header_count, field_count;
//    int status_code;

    // Initialise a map to store name-value pair
    sds_map_t *map = malloc(sizeof(*map));
    map_init(map);

    // Tokenize header into lines
    sds *lines = sdssplitlen(header, sdslen(header), "\r\n", 2, &header_count);

    for (int j = 0; j < header_count; j++) {
        if (j == 0) {
            // Split status line
            status_line = sdssplitlen(lines[j], sdslen(lines[j]), " ", 1, &field_count);
            // convert status code to int
//            status_code = atoi(status_line[1]);
            sdsfreesplitres(status_line, field_count);
        } else {
            // Locate the first colon
            char *first_colon = strstr(lines[j], ":");
            sds name = sdscpylen(sdsempty(), lines[j], strlen(lines[j]) - strlen(first_colon));
            sds value = sdsnew(first_colon + 1);

            // Add header to map
            map_set(map, lower(name), sdstrim(value, " \n"));
            sdsfree(name);
            sdsfree(value);
        }
    }
    sdsfreesplitres(lines, header_count);
    sdsfree(header);
    return map;
}


bool isBufferSufficient(sds_map_t *header_map) {
    sds content_length = getContentLength(header_map);
    if (content_length) {

        int expected = 0;
        if (!parse_int(content_length, &expected)) {
            return false;
        }
        if (expected > RESPONSE_BUFFER - 1) {
            log_error("\t|- Aborted: Expected content length exceeds buffer size");
            return false;
        }
    }
    return true;
}

bool parse_int(sds string, int *parse_result) {
    errno = 0;
    char *ptr = NULL;
    *parse_result = (int) strtol(string, &ptr, 10);
    if (errno) {
        log_error("Parsing integer failed");
        return false;
    }
    return true;
}

bool isHTML(sds_map_t *header_map) {
    sds type = getContentType(header_map);
    // if content type header is presented
    if (type) {
        if (!strstr(type, HTML_CONTENT_TYPE)) {
            log_error("\t|- Aborted: Content type is not HTML");
            return false;
        }
    }
    // otherwise return ok
    return true;
}