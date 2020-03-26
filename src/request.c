//
// Created by Haswe on 3/20/2020.

#include "request.h"
#include "config.h"

/**
 * Create a HTTP Request
 * @param host
 * @param path
 * @param method
 * @param vec_header_p
 * @param body
 * @return
 */
Request *create_http_request(sds host, sds path, sds method, sds body) {
    Request *request = malloc(sizeof(Request));
    assert(request != NULL);
    request->method = method;
    request->path = path;
    request->version = sdsnew(HTTP_VERSION);
    sds_map_t *header = malloc(sizeof(*header));
    map_init(header);
    request->header = header;
    request->body = body;
    request->host = host;
    add_header(request, "Host", host);
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
    const char *key;
    map_iter_t iter = map_iter(response->header);
    while ((key = map_next(req->header, &iter))) {
        reqString = sdscat(reqString, sdscatprintf(sdsempty(), "%s: %s\r\n", key, *map_get(req->header, key)));
    }
    reqString = sdscat(reqString, "\r\n");
    reqString = sdscat(reqString, req->body);
    return reqString;
}


/**
 * Add a header to request
 * @param req
 * @param name
 * @param value
 * @return
 */
int add_header(Request *req, char *name, char *value) {
    map_set(req->header, name, value);
    return 0;
}

/**
 * Free memory allocated to a Request
 * @param req
 * @return
 */
int free_request(Request *req) {
    sdsfree(req->method);
    req->method = NULL;
    sdsfree(req->host);
    req->host = NULL;
    sdsfree(req->path);
    req->path = NULL;
    sdsfree(req->version);
    req->version = NULL;
    sdsfree(req->body);
    req->body = NULL;
    map_deinit(req->header);
    free(req);;
}
