//
// Created by Haswe on 3/23/2020.
//

#include "response.h"
#include "../lib/pbl.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

Response *parseHTTPResponse(char *message) {
    Response *response = (Response *) malloc(sizeof(*response));
    int status;
    int header_size = strlen(message) - strlen(strstr(message, "\r\n\r\n"));
    int body_size = strlen(message) - header_size;

    char header[header_size];
    char body[body_size];

    memset(header, 0, header_size);
    memset(body, 0, body_size);

    // TODO:　Try to split headers and body with sdssplitlen()
    status = strncpy(header, message, header_size);
    // TODO: Figure out how many space body really needs
    status = strncpy(body, strstr(message, "\r\n\r\n") + 4, body_size);
    sds *fields, *status_line, *split;
    int header_count, field_count, split_count;

    sds line = sdsnew(header);

    fields = sdssplitlen(line, sdslen(line), "\r\n", 2, &header_count);

    for (int j = 0; j < header_count; j++) {
        if (j == 0) {
            status_line = sdssplitlen(sdsnew(fields[j]), sdslen(fields[j]), " ", 1, &field_count);
            response->version = sdsnew(status_line[0]);
            response->status_code = atoi(status_line[1]);
            response->reason_phrase = sdsnew(status_line[2]);
        } else {
            split = sdssplitlen(sdsnew(fields[j]), sdslen(fields[j]), ":", 1, &split_count);
            sds name = split[0];
            sds value = sdstrim(split[1], " \n");
            printf("Name: %s\tValue=%s %d\n", name, value, sdscmp(name, sdsnew("Content-Length")));
        }
    }

    sdsfreesplitres(fields, header_count);
//    status = strncpy(header, message, 10);

//    printf("-----------------------\n%s\n----------------------", body);
}
