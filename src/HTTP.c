
/**
 * Module to handle HTTP request and response
 * Created by Shuyang Fan on 3/20/2020.
 */


#include "HTTP.h"
/**
 * Send a HTTP request
 * @param request
 * @param portno the connection used to send request
 * @param error
 * @return
 */
response_t *send_http_request(request_t *request, int portno, int *error) {
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