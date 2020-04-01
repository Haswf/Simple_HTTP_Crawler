//
// Created by Haswe on 3/22/2020.
//

#include "connection.h"
#include "crawler.h"

int create_connection(sds host, int portno, int *sockfd) {
    struct hostent *server;
    struct sockaddr_in serv_addr;
    /* lookup the ip address */
    server = gethostbyname(host);
    if (server == NULL) {
        log_error("Failed to find host: %s", host);
        return 1;
    }

    /* create the socket */
    *sockfd = socket(AF_INET, SOCK_STREAM, 0);

    if (*sockfd < 0) {
        log_error("Failed to create socket: %s", host);
        return 1;
    }

    /* fill in the structure */
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(portno);
    memcpy(&serv_addr.sin_addr.s_addr, server->h_addr, server->h_length);

    /* connect the socket */
    if (connect(*sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
        log_error("Failed to connect to socket: %d", *sockfd);
        return 1;
    }
    // Disable socket timeout
//    if (set_timeout(*sockfd)) {
//        return 1;
//    }

    return 0;
}

int send_to_server(int sockfd, sds message) {
    int total, sent, bytes;
    sent = 0;
    total = sdslen(message);
    /* send the request */
    do {
        bytes = write(sockfd, message + sent, total - sent);
        if (bytes < 0) {
            log_error("failed to write message to socket: %d %s", sockfd, message + sent);
            // return 1 to indicate an error
            return 1;
        }
        if (bytes == 0)
            break;
        sent += bytes;
    } while (sent < total);
    return 0;
}

int close_connection(int sockfd) {
    /* close the socket */
    close(sockfd);
    return 0;
}

int receive_from_server(int sockfd, sds *buffer) {
    int total, received, bytes, content_total;

    char *body_pos = NULL;
    sds_map_t *header_map = NULL;

    /* receive the response */
    total = RESPONSE_BUFFER - 1;
    received = 0;
    content_total = 0;

    bool has_content_length = false;

    while (received < total) {
        bytes = recv(sockfd, *buffer + received, total - received, 0);
        if (bytes < 0) {
            log_error("recv() failed or connection closed prematurely");
            return 1;
        }
        if (bytes == 0)
            break;
        received += bytes;

        // If we find header for the first time
        if (locate_body(*buffer) && !body_pos) {
            body_pos = locate_body(*buffer);
            header_map = extract_header(*buffer);
            if (!isBufferSufficient(header_map) || !isHTML(header_map)) {
                return 1;
            };
            if (getContentLength(header_map)) {
                content_total = atoi(getContentLength(header_map));
                has_content_length = true;
            }
        }

        if (has_content_length) {
            if (strlen(body_pos) >= content_total) {
                break;
            }
        }


    }

    if (received == total) {
        log_error("Fail to save response to buffer: BUFFER FULL");
        return 1;
    }

    if (header_map) {
        map_deinit(header_map);
        free(header_map);
    }
    return 0;
}

int set_timeout(int sockfd) {
    struct timeval timeout;
    timeout.tv_sec = 10;
    timeout.tv_usec = 0;

    if (setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (char *) &timeout,
                   sizeof(timeout)) < 0) {
        log_error("setsockopt failed");
        return 1;

    }

    if (setsockopt(sockfd, SOL_SOCKET, SO_SNDTIMEO, (char *) &timeout,
                   sizeof(timeout)) < 0) {
        log_error("setsockopt failed");
        return 1;

    }
    return 0;
}