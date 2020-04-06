/**
 * Module to handle connection.
 * Created by Shuyang Fan on 3/22/2020.
 */


#ifndef COMP30023_2020_PROJECT1_CONNECTION_H
#define COMP30023_2020_PROJECT1_CONNECTION_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <time.h>
#include <stdbool.h>

#include "parser.h"
#include "collection.h"
#include "config.h"

#define h_addr h_addr_list[0] /* for backward compatibility */

int create_connection(sds host, int portno, int *sockfd);

int send_to_server(int sockfd, char *message);

int close_connection(int sockfd);

int receive_from_server(int sockfd, sds *buffer);

int set_timeout(int socketfd);


#endif //COMP30023_2020_PROJECT1_CONNECTION_H
