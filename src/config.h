//
// Created by Haswe on 3/26/2020.
//

#ifndef COMP30023_2020_PROJECT1_CONFIG_H
#define COMP30023_2020_PROJECT1_CONFIG_H

#define PORT 80
#define LOG_LEVEL LOG_TRACE
#define HEADER_BODY_SEPARATOR_SIZE 4
#define HTTP_VERSION "HTTP/1.1"
#define USER_AGENT "shuyangf"
#define CONNECTION "close"

/*
 * The size of response buffer.
 * No server response will be longer than 100,000 bytes
 */
#define RESPONSE_BUFFER 100001

#define FAILURE_FLAG (-1)
#define RETRY_FLAG 0
#define VISITED_FLAG 1
#define POST_FLAG 2
#define SERVICE_UNAVAILABLE 503
#define GATEWAY_TIMEOUT 504
#define NOT_FOUND 404
#define GONE 410
#define URI_TOO_LONG 414

#define CONTENT_TYPE "content-type"
#define HTML_CONTENT_TYPE "text/html"
#define CONTENT_LENGTH "content-length"
#define CONTENT_LOCATION "content-location"

#define LOCATION "location"
#endif //COMP30023_2020_PROJECT1_CONFIG_H
