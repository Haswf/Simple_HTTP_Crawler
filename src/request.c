//
// Created by Haswe on 3/20/2020.

#include "request.h"

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
    // Return NULL if malloc fails
    if (!request) {
        return NULL;
    }

    // join host and path for url parsing
    request->parsed_url = parse_url(sdscatprintf(sdsempty(), "http://%s%s", host, path));

    if (!request->parsed_url) {
        return NULL;
    }

    request->body = body;
    request->host = host;
    request->method = method;
    request->path = path;
    request->version = sdsnew(HTTP_VERSION);
    sds_map_t *header = malloc(sizeof(*header));
    map_init(header);
    request->header = header;
    if (!request->header) {
        return NULL;
    }

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
    return map_set(req->header, name, value);
}

/**
 * Free memory allocated to a Request
 * @param req
 * @return
 */
int free_request(Request *req) {
    if (req->method) {
        sdsfree(req->method);
        req->method = NULL;
    }
    if (req->host) {
        sdsfree(req->host);
        req->host = NULL;
    }
    if (req->path) {
        sdsfree(req->path);
        req->path = NULL;
    }
    if (req->version) {
        sdsfree(req->version);
        req->version = NULL;
    }
    if (req->body) {
        sdsfree(req->body);
        req->body = NULL;
    }
    if (req->header) {
        map_deinit(req->header);
        req->header = NULL;
    }
    free(req);;
}
