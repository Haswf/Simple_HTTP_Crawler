//
// Created by Haswe on 3/22/2020.
//

#ifndef COMP30023_2020_PROJECT1_CONNECTION_H
#define COMP30023_2020_PROJECT1_CONNECTION_H

#include <stdio.h> /* printf, sprintf */
#include <stdlib.h> /* exit, atoi, malloc, free */
#include <unistd.h> /* read, write, close */
#include <string.h> /* memcpy, memset */
#include <sys/socket.h> /* socket, connect */
#include <netinet/in.h> /* struct sockaddr_in, struct sockaddr */
#include <netdb.h> /* struct hostent, gethostbyname */
#include <time.h>
#include "../lib/sds.h"

int create_connection(sds host, int portno);

int send_to_server(int sockfd, char *message);

int close_connection(int sockfd);

char *receive_from_server(int sockfd);

#endif //COMP30023_2020_PROJECT1_CONNECTION_H
