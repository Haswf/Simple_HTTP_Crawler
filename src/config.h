//
// Created by Haswe on 3/26/2020.
//

#ifndef COMP30023_2020_PROJECT1_CONFIG_H
#define COMP30023_2020_PROJECT1_CONFIG_H

#define PORT 80
/**
 * Log level
 * LOG_ERROR　ｗill print all error to stderr
 * LOG_INFO will print all url fetched and result to stderr
 * LOG_DEBUG will print more details, etc found urls to stderr.
 * LOG_TRACE will print everything including request and response to stderr.
 */
#define LOG_LEVEL LOG_TRACE

/**
 * The length of "/r/n/r/n"
 */
#define HEADER_BODY_SEPARATOR_SIZE 4


/*
 * The size of response buffer.
 * No server response will be longer than 100,000 bytes
 */
#define RESPONSE_BUFFER 100001

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

/**
 * HTTP headers
 */
#define CONTENT_TYPE "content-type"
#define HTML_CONTENT_TYPE "text/html"
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
#endif //COMP30023_2020_PROJECT1_CONFIG_H
