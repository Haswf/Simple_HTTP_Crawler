//
// Created by Haswe on 3/23/2020.
//

#ifndef COMP30023_2020_PROJECT1_RESPONSE_H
#define COMP30023_2020_PROJECT1_RESPONSE_H

#include "../lib/sds.h"
#include "../lib/pbl.h"

typedef struct Response {
    sds version;
    int status_code;
    sds reason_phrase;
    sds body;
    PblMap *header;
} Response;

Response *parseHTTPResponse(char *message);

#endif //COMP30023_2020_PROJECT1_RESPONSE_H
