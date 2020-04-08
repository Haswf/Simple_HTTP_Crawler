/**
 * Configuration of the crawler.
 * Created by Haswe on 3/26/2020.
 */

#ifndef COMP30023_2020_PROJECT1_CONFIG_H
#define COMP30023_2020_PROJECT1_CONFIG_H

#include "../lib/log/log.h"

/**
 * Configuration
 */

/**
 * Destination Port number
 */
#define PORT 80

/**
 * Log level
 * LOG_ERROR　ｗill print all error to stderr
 * LOG_INFO will print all url fetched and result to stderr
 * LOG_DEBUG will print more details, etc found urls to stderr.
 * LOG_TRACE will print everything including request and response to stderr.
 */
#define LOG_LEVEL LOG_DEBUG

/* The size of response buffer. No server response will be longer than 100,000 bytes */
#define RESPONSE_BUFFER 100001

/* Allowed content type */
#define ALLOWED_CONTENT_TYPE HTML_ONLY
/* Options */
#define HTML_ONLY "text/html"

/* Cross domain fetching */
#define CROSS_DOMAIN_POLICY SECOND_LEVEL_DOMAIN
/* Options */
/* SAME_DOMAIN follow urls with exact same domain
 * SECOND_LEVEL_DOMAIN follow urls with exact same domain except second level domain
 */
#define SAME_DOMAIN 0
#define SECOND_LEVEL_DOMAIN 1

/*
 * Content-length validation
 */
#define CONTENT_LENGTH_POLICY STRICT
/* Options
 * STRICT Response without content-header will be discarded
 * ALL Response without content-header will be parsed
 * */
#define STRICT 1


/* Scheme Policy */
#define SCHEME_POLICY HTTP_ONLY
#define HTTP_ONLY "http"
#define ALL NULL


/**
 * Flag indicating what to do next with a url
 */
#define FAILURE_FLAG (-1)
#define RETRY_FLAG 0
#define VISITED_FLAG 1
#define POST_FLAG 2
#define AUTH_FLAG 3

/**
 * Base64 Content for Authorization header
 */
#define TOKEN "c2h1eWFuZ2Y6cGFzc3dvcmQ="

/**
 * HTTP status code
 */

#define UNAUTHORISED 401
#define NOT_FOUND 404
#define GONE 410
#define URI_TOO_LONG 414
#define SERVICE_UNAVAILABLE 503
#define GATEWAY_TIMEOUT 504


/*
 * MIME types
 */


/**
 * HTTP headers
 */
#define CONTENT_TYPE "content-type"
#define CONTENT_LENGTH "content-length"
#define CONTENT_LOCATION "content-location"
#define AUTHORIZATION "authorization"
#define LOCATION "location"

/**
 * HTTP Header value
 */
#define HTTP_VERSION "HTTP/1.1"
#define USER_AGENT "shuyangf"
#define CONNECTION "close"

#define ERROR 1
#define SUCCESS 0

/* The length of "/r/n/r/n" */
#define HEADER_BODY_SEPARATOR_SIZE 4
#endif //COMP30023_2020_PROJECT1_CONFIG_H
