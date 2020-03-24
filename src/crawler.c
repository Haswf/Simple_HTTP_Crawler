//
// Created by Haswe on 3/25/2020.
//

#include "crawler.h"
#include "request.h"
#include "response.h"
#include "HTTP.h"
#include "parser.h"

#define PORT 80

int main(int agrc, char *argv[]) {

    Request *request = create_http_request(sdsnew("www.pcre.org"), sdsnew("/"), sdsnew("GET"), sdsnew(""));
    add_header(request, "Connection", "close");
    add_header(request, "User-Agent", "shuyangf");

    Response *response = send_http_request(request, PORT);
//    free_request(request);
//    request = NULL;

    print_header(response);
    print_body(response);
    print_url(response);
    free_response(response);
//    resolve_relative_path();
}