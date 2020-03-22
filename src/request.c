//
// Created by Haswe on 3/20/2020.

#include "request.h"

#define RESPONSE_BUFFER 4096

/**
 * Create a HTTP Request
 * @param host
 * @param path
 * @param method
 * @param vec_header_p
 * @param body
 * @return
 */
Request *createHTTPRequest(sds host, sds path, sds method, vec_header_t *vec_header_p, sds body) {
    Request *request = calloc(1, sizeof(Request));
    request->method = method;
    request->path = path;
    request->version = "HTTP/1.1";
    request->headers = vec_header_p;
    request->body = body;
    request->host = host;
    appendHeader(request, "Host", host);
    return request;
};

/**
 * Convert a HTTP Request to a string
 * @param req
 * @return
 */
sds HTTPRequestToString(Request *req) {
    sds reqString = sdsempty();
    reqString = sdscat(reqString, sdscatprintf(sdsempty(), "%s %s %s\r\n", req->method, req->path, req->version));
    /* Iterates and prints the value and index of each value in the float vec */
    int i;
    Header *header;
    vec_foreach(req->headers, header, i) {
            sds headerString = sdscatprintf(sdsempty(), "%s: %s\r\n", header->name, header->value);
            reqString = sdscat(reqString, headerString);
        }
    reqString = sdscat(reqString, "\r\n");
    reqString = sdscat(reqString, req->body);
    return reqString;
}

/**
 * Create a header
 * @param name
 * @param value
 * @return
 */
Header *createHeader(sds name, sds value) {
    Header *header = calloc(1, sizeof(Header));
    header->name = name;
    header->value = value;
    return header;
}

/**
 * Add a header to request
 * @param req
 * @param name
 * @param value
 * @return
 */
int appendHeader(Request *req, char *name, char *value) {
    Header *new = createHeader(name, value);
    vec_push(req->headers, new);
    return 0;
}


/**
 * Free memory allocated to a header
 * @param header
 * @return
 */
int freeHeader(Header *header) {
    free(header);
    if (header == NULL) {
        return 0;
    } else {
        return -1;
    }
}

/**
 * Free memory allocated to a Request
 * @param req
 * @return
 */
int freeRequest(Request *req) {
    sdsfree(req->path);
    sdsfree(req->method);
    sdsfree(req->version);
    sdsfree(req->body);
    int i;
    Header *header;
    vec_foreach(req->headers, header, i) {
            freeHeader(header);
        };
    vec_deinit(req->headers);
    free(req);;
}