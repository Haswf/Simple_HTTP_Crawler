/**
 * Module to handle connection.
 * Created by Shuyang Fan on 3/22/2020.
 */

#include "connection.h"

/**
 * Establish a connection with a host with given portno
 * @param host
 * @param portno
 * @param sockfd a pointer to integer where sockdf will be written to.
 * @return error indicator as defined in config.h
 */
int create_connection(sds host, int portno, int *sockfd) {
    struct hostent *server;
    struct sockaddr_in serv_addr;
    /* lookup the ip address */
    server = gethostbyname(host);

    /* if no such host exist */
    if (server == NULL) {
        log_error("Failed to find host: %s", host);
        return ERROR;
    }

    /* create the socket */
    *sockfd = socket(AF_INET, SOCK_STREAM, 0);

    if (*sockfd < 0) {
        log_error("Failed to create socket: %s", host);
        return ERROR;
    }

    /* fill in the structure */
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(portno);
    memcpy(&serv_addr.sin_addr.s_addr, server->h_addr, server->h_length);

    /* connect the socket */
    if (connect(*sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
        log_error("Failed to connect to socket: %d", *sockfd);
        return ERROR;
    }
    // Disable socket timeout
//    if (set_timeout(*sockfd)) {
//        return 1;
//    }

    return SUCCESS;
}

/**
 * Send a message to server
 * @param sockfd socket file descriptor returned by create_connection
 * @param message message to be sent
 * @return error indicator as defined in config.h
 * */
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
            return ERROR;
        }
        if (bytes == 0)
            break;
        sent += bytes;
    } while (sent < total);
    return SUCCESS;
}

/**
 * Close connection
 * @param sockfd
 * @return error indicator as defined in config.h
 */
int close_connection(int sockfd) {
    /* close the socket */
    close(sockfd);
    return SUCCESS;
}

/**
 * Receive response from server
 * @param sockfd sock file descriptor to receive
 * @param buffer save response to this buffer
 * @return error indicator as defined in config.h
 */
int receive_from_server(int sockfd, sds *buffer) {
    int total, received, bytes, content_total;

    char *body_pos = NULL;
    sds_map_t *header_map = NULL;

    /* Leave one byte to save new line character */
    total = RESPONSE_BUFFER - 1;
    received = 0;
    content_total = 0;
    bytes = 0;
    content_total = 0;

    /* if we have received the content length header or not */
    bool has_content_length = false;

    /* receive the response */
    while (received < total) {
        bytes = recv(sockfd, *buffer + received, total - received, 0);
        /* Return ERROR if recv encounter an error */
        if (bytes < 0) {
            log_error("recv() failed or connection closed by peer");
            return ERROR;
        }
        /*
         * Break the loop if no further bytes is available
         */
        if (bytes == 0)
            break;
        received += bytes;

        /* If we find header for the first time */
        if (locate_body(*buffer) && !body_pos) {
            body_pos = locate_body(*buffer);
            header_map = extract_header(*buffer);
            /* Check if buffer is sufficent to store response
             * Check if the MIME type is HTML
             * */
            if (!isBufferSufficient(header_map) || !isHTML(header_map)) {
                map_deinit(header_map);
                free(header_map);
                return ERROR;
            };
            /* Retrieve content length from header */
            if (getContentLength(header_map)) {
                content_total = atoi(getContentLength(header_map));
                has_content_length = true;
            }
        }
        /*
         * Keep reading until content length bytes have been read
         */
        if (has_content_length) {
            if (strlen(body_pos) >= content_total) {
                break;
            }
        }
    }
    /*
     * Return error if buffer is full
     */
    if (received == total) {
        log_error("Fail to save response to buffer: BUFFER FULL");
        return ERROR;
    }
    /*
     * Clean up
     */
    if (header_map) {
        map_deinit(header_map);
        free(header_map);
    }
    return SUCCESS;
}

/**
 * Set socket timeout. The following code was adapted from Toby's answer to
 * https://stackoverflow.com/questions/4181784/how-to-set-socket-timeout-in-c-when-making-multiple-connections
 * @param sockfd socket file descriptor
 * @return
 */
int set_timeout(int sockfd) {
    struct timeval timeout;
    /*
     * 10 seconds timeout
     */
    timeout.tv_sec = 10;
    timeout.tv_usec = 0;

    /*
     * Receive timeout
     */
    if (setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (char *) &timeout,
                   sizeof(timeout)) < 0) {
        log_error("setsockopt failed");
        return ERROR;

    }

    /*
     * Send timeout
    */
    if (setsockopt(sockfd, SOL_SOCKET, SO_SNDTIMEO, (char *) &timeout,
                   sizeof(timeout)) < 0) {
        log_error("setsockopt failed");
        return ERROR;

    }
    return SUCCESS;
}