//
// Created by Haswell on 3/20/2020.
//


#include "HTTP.h"

#define RESPONSE_BUFFER 100001
#define PORT 80

void error(const char *msg) {
    perror(msg);
    exit(0);
}

int main(int argc, char *argv[]) {
    sds host = sdsnew("google.com");
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

    struct hostent *server;
    struct sockaddr_in serv_addr;
    int sockfd, bytes, sent, received, total, message_size;
    char *message, response[RESPONSE_BUFFER];

    message_size = sdslen(req);
    /* allocate space for the message */
    message = malloc(message_size);
    sprintf(message, "%s", req);

    /* create the socket */
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) error("ERROR opening socket");

    /* lookup the ip address */
    server = gethostbyname(host);
    if (server == NULL) error("ERROR, no such host");

    /* fill in the structure */
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(portno);
    memcpy(&serv_addr.sin_addr.s_addr, server->h_addr, server->h_length);

    /* connect the socket */
    if (connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
        error("ERROR connecting");

    /* send the request */
    total = strlen(message);
    sent = 0;
    printf("Sending message\n");
    do {
        bytes = write(sockfd, message + sent, total - sent);
        if (bytes < 0)
            error("ERROR writing message to socket\n");
        if (bytes == 0)
            break;
        sent += bytes;
        printf("%d/%d bytes sent\n", sent, total);
    } while (sent < total);

    /* receive the response */
    memset(response, 0, sizeof(response));
    total = sizeof(response) - 1;
    received = 0;

    while (received < total) {
        bytes = recv(sockfd, response + received, total - received, 0);
        printf("%d/%d bytes received\t%d/%d bytes free \n", bytes, bytes + received, total - received, total);
        /* Receive up to the buffer size (minus 1 to leave space for
           a null terminator) bytes from the sender */
        if (bytes < 0)
            error("recv() failed or connection closed prematurely");
        if (bytes == 0)
            break;
        received += bytes;   /* Keep tally of total bytes */
    }

    if (received == total)
        error("ERROR storing complete response from socket");

    /* close the socket */
    close(sockfd);

    /* process response */
    printf("\nResponse:\n%s\n", response);

    free(message);
    return 0;
}

