//
// Created by Haswell on 3/20/2020.
//

#include "crawler.h"

#define RESPONSE_BUFFER 100001

void error(const char *msg) {
    perror(msg);
    exit(0);
}


Response *send_http_request(Request *request, int portno) {
    // Create a connection
    int sockfd = create_connection(request->host, portno);
    // Convert request to a string
    sds reqString = HTTPRequestToString(request);
    send_to_server(sockfd, reqString);
    sdsfree(reqString);

    sds buffer = sdsnewlen("", RESPONSE_BUFFER);
    receive_from_server(sockfd, &buffer);
    close_connection(sockfd);
    Response *response = parse_response(&buffer);
    sdsfree(buffer);
    return response;
}

