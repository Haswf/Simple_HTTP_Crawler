//
// Created by Haswe on 3/22/2020.
//

#include "connection.h"
#include "HTTP.h"

#define RESPONSE_BUFFER 100001

int create_connection(sds host, int portno) {
    struct hostent *server;
    struct sockaddr_in serv_addr;
    int sockfd, bytes, sent, received, total, message_size;
    /* lookup the ip address */
    server = gethostbyname(host);
    if (server == NULL) error("ERROR, no such host");

    /* create the socket */
    sockfd = socket(AF_INET, SOCK_STREAM, 0);

    if (sockfd < 0) error("ERROR opening socket");

    /* fill in the structure */
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(portno);
    memcpy(&serv_addr.sin_addr.s_addr, server->h_addr, server->h_length);

    /* connect the socket */
    if (connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
        error("ERROR connecting");
    return sockfd;
}

int send_to_server(int sockfd, sds message) {
    int total, sent, bytes;
    sent = 0;
    total = strlen(message);
    /* send the request */
//    printf("Sending message\n");
    do {
        bytes = write(sockfd, message + sent, total - sent);
        if (bytes < 0)
            error("ERROR writing message to socket\n");
        if (bytes == 0)
            break;
        sent += bytes;
//        printf("%d/%d bytes sent\n", sent, total);
    } while (sent < total);
    return sent;
}

int close_connection(int sockfd) {
    /* close the socket */
    close(sockfd);
    return 0;
}

void receive_from_server(int sockfd, sds *buffer) {
    int total, received, bytes;

    /* receive the response */
    total = RESPONSE_BUFFER - 1;
    received = 0;

    while (received < total) {
        bytes = recv(sockfd, *buffer + received, total - received, 0);
//        printf("%d/%d bytes received\t%d/%d bytes free \n", bytes, bytes + received, total - received, total);
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
    sdsRemoveFreeSpace(*buffer);
}