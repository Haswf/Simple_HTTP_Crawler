//
// Created by Haswell on 3/20/2020.
//

#include "connection.h"
#include "request.h"
#include "response.h"

#define RESPONSE_BUFFER 100001

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
    *error = send_to_server(sockfd, reqString);
    if (*error != 0) {
        sdsfree(reqString);
        return NULL;
    } else {
        sdsfree(reqString);
    }
    sds buffer = sdsnewlen("", RESPONSE_BUFFER);
    *error = receive_from_server(sockfd, &buffer);
    *error = close_connection(sockfd);
    // free response buffer if anything goes wrong
    if (*error != 0) {
        sdsfree(buffer);
        return NULL;
    } else {
        response = parse_response(&buffer);
        sdsfree(buffer);
        return response;
    }
}

