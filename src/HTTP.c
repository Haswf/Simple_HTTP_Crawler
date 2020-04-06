
/**
 * Module to handle HTTP request and response
 * Created by Shuyang Fan on 3/20/2020.
 */

#include <stdbool.h>
#include "connection.h"
#include "request.h"
#include "response.h"
#include "config.h"
#include "crawler.h"
#include <errno.h>

/**
 * Send a HTTP request
 * @param request
 * @param portno the connection used to send request
 * @param error
 * @return
 */
response_t *send_http_request(Request *request, int portno, int *error) {
    // Socket file descriptor
    int sockfd = 0;
    response_t *response = NULL;
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
    bzero(buffer, RESPONSE_BUFFER);

    *error = receive_from_server(sockfd, &buffer);
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
 * Check if the full header has been received by looking for \r\n\r\n
 * Return NULL if not yet received
 * @param buffer
 * @return char* pointing to the head of the body
 */
char *locate_body(char *buffer) {
    char *header_body_separator = "\r\n\r\n";
    return strstr(buffer, header_body_separator);
}

/**
 * Extract headers from response as a map
 * @param buffer
 * @return sds_map_t
 */
sds_map_t *extract_header(char *buffer) {
    /* Search for the body */
    char *where_body_is = locate_body(buffer);
    if (!where_body_is) {
        return NULL;
    }
    /*
     * Determine the header length
     */
    int header_size = (int) strlen(buffer) - (int) strlen(where_body_is);
    sds header = sdscpylen(sdsempty(), buffer, header_size);
    sds *status_line;
    int header_count, field_count;

    /* Initialise a map */
    sds_map_t *map = malloc(sizeof(*map));
    map_init(map);

    /* Tokenize header into lines */
    sds *lines = sdssplitlen(header, sdslen(header), "\r\n", 2, &header_count);

    for (int j = 0; j < header_count; j++) {
        // Split status line
        if (j == 0) {
            status_line = sdssplitlen(lines[j], sdslen(lines[j]), " ", 1, &field_count);
            sdsfreesplitres(status_line, field_count);
        } else {
            // Locate the first colon
            char *first_colon = strstr(lines[j], ":");
            sds name = sdscpylen(sdsempty(), lines[j], strlen(lines[j]) - strlen(first_colon));
            sds value = sdsnew(first_colon + 1);
            sdstrim(value, " \n");

            // Add header to map
            map_set(map, lower(name), value);
            sdsfree(name);
//            sdsfree(value);
        }
    }
    sdsfreesplitres(lines, header_count);
    sdsfree(header);
    return map;
}

/**
 * Determines if buffer is large enough to receive full response
 * @param header_map
 * @return
 */
bool isBufferSufficient(sds_map_t *header_map) {
    sds content_length = getContentLength(header_map);
    if (content_length) {

        int expected = 0;
        if (!parse_int(content_length, &expected)) {
            return false;
        }
        if (expected > RESPONSE_BUFFER - 1) {
            log_error("\t|- Aborted: Expected Content-Length exceeds buffer size %d > %d", expected, RESPONSE_BUFFER);
            return false;
        }
    }
    return true;
}

/**
 * A wrapper of strtol with proper logging
 * @param string
 * @param parse_result
 * @return
 */
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

/**
 * Check if the MIME type is html
 * @param header_map
 * @return
 */
bool isHTML(sds_map_t *header_map) {
    sds type = getContentType(header_map);
    // if content type header is presented
    if (type) {
        if (!strstr(type, HTML_CONTENT_TYPE)) {
            log_error("\t|- Aborted: Content type is not HTML");
            return false;
        }
    }
    // otherwise return true
    return true;
}