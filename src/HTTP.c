//
// Created by Haswell on 3/20/2020.
//


#include "HTTP.h"
#include "connection.h"
#include "response.h"

#define RESPONSE_BUFFER 100001
#define PORT 80

void error(const char *msg) {
    perror(msg);
    exit(0);
}


int main(int argc, char *argv[]) {
    sds host = sdsnew("www.pcre.org");
    sds path = sdsnew("/");
    sds method = sdsnew("GET");
    printf("Fetching URL %s %s\n", host, path);
    int portno = PORT;

    vec_header_t v;
    vec_init(&v);

    Request *req_p = createHTTPRequest(host, path, method, &v, "");
    appendHeader(req_p, "Connection", "close");
    appendHeader(req_p, "User-Agent", "shuyangf");
    sds req = HTTPRequestToString(req_p);
    printf("Request to send\n\n%s\n", req);

    int sockfd;

    sockfd = create_connection(host, portno);
    send_to_server(sockfd, req);

    char *response = receive_from_server(sockfd);

//    printf("\nResponse:\n%s\n", response);

    parseHTTPResponse(response);

    close_connection(sockfd);
////    sds test = sdsnew("Hello");
//    sdsfree(req_p->method);
//    free(response);
//    freeRequest(req);
//    sdsfree(req);
    return 0;
}

