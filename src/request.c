/**
 * Module to handle HTTP request
 * Created by Shuyang Fan on 3/23/2020.
 */

#include "request.h"

/**
 * Create a HTTP request
 * @param host
 * @param path
 * @param method
 * @param vec_header_p
 * @param body
 * @return
 */
request_t *create_http_request(sds host, sds path, sds method, sds body) {
    request_t *request = malloc(sizeof(*request));
    // Return NULL if malloc fails
    if (!request) {
        return NULL;
    }

    /*
     * Join host and path for url parsing
     */
    sds join = sdscatprintf(sdsempty(), "http://%s%s", host, path);
    request->parsed_url = parse_url(join);
    sdsfree(join);

    /*
     * Return null if parsing url failed
     */
    if (!request->parsed_url) {
        return NULL;
    }

    request->body = body;
    request->host = host;
    request->method = method;
    request->path = path;
    request->version = sdsnew(HTTP_VERSION);

    /* Allocate space for map */
    request->header = malloc(sizeof(*request->header));
    // Return NULL if malloc failed
    if (!request->header) {
        return NULL;
    }
    map_init(request->header);

    /* Add host to header as requested by HTTP 1.1 */
    add_header(request, "Host", host);
    return request;
};

/**
 * Convert a HTTP Request to a string
 * @param req
 * @return
 */
sds HTTPRequestToString(request_t *req) {
    sds reqString = sdsempty();
    reqString = sdscatprintf(reqString, "%s %s %s\r\n", req->method, req->path, req->version);
    const char *key;
    map_iter_t iter = map_iter(response->header);
    while ((key = map_next(req->header, &iter))) {
        reqString = sdscatprintf(reqString, "%s: %s\r\n", key, *map_get(req->header, key));
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
int add_header(request_t *req, char *name, char *value) {
    return map_set(req->header, name, value);
}

/**
 * Free memory allocated to a Request
 * @param req
 * @return
 */
int free_request(request_t *req) {
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
        free(req->header);
        req->header = NULL;
    }
    if (req->parsed_url) {
        free_url(req->parsed_url);
        req->header = NULL;
    }
    free(req);
    return SUCCESS;
}
