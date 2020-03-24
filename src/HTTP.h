//
// Created by Haswe on 3/22/2020.
//

#ifndef COMP30023_2020_PROJECT1_HTTP_H
#define COMP30023_2020_PROJECT1_HTTP_H

#include <stdio.h> /* printf, sprintf */
#include <stdlib.h> /* exit, atoi, malloc, free */
#include <unistd.h> /* read, write, close */
#include <string.h> /* memcpy, memset */
#include <sys/socket.h> /* socket, connect */
#include <netinet/in.h> /* struct sockaddr_in, struct sockaddr */
#include <netdb.h> /* struct hostent, gethostbyname */
#include <time.h>
#include "request.h"
#include "response.h"

void error(const char *msg);

Response *send_http_request(Request *request, int portno);

#endif //COMP30023_2020_PROJECT1_HTTP_H
