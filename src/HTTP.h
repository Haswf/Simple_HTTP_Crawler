//
// Created by Haswe on 3/22/2020.
//

#ifndef COMP30023_2020_PROJECT1_HTTP_H
#define COMP30023_2020_PROJECT1_HTTP_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <errno.h>
#include "connection.h"
#include "response.h"
#include "request.h"
#include "config.h"

response_t *send_http_request(request_t *request, int portno, int *status);

#endif //COMP30023_2020_PROJECT1_HTTP_H
