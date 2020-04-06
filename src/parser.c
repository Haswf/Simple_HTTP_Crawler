/**
 * Module to parse html with Gumbo
 * Created by Shuyang Fan on 3/25/2020.
 */

#include "parser.h"
#include "crawler.h"


/**
 * Parse the html and add urls to the job queue
 * @param url_parse current url, this gives a context to resolve reference
 * @param html html to be parsed
 * @param job_queue
 * @param seen
 */
void search_and_add_url(url_t *url_parse, sds html, sds_vec_t *job_queue, int_map_t *seen) {
    GumboOutput *output = gumbo_parse(html);
    add_to_job_queue(url_parse, output->root, job_queue, seen);
    gumbo_destroy_output(&kGumboDefaultOptions, output);
}

/**
 * Helper function to validate and resolve reference
 * @param url_parse
 * @param node
 * @param job_queue
 * @param seen
 */
void add_to_job_queue(url_t *url_parse, GumboNode *node, sds_vec_t *job_queue, int_map_t *seen) {
    /**
     * The following code is adapted from Gumbo example code, which can be obtained at
     * https://github.com/google/gumbo-parser/blob/master/examples/find_links.cc
     * Some modification has been made to add found url to job queue.
     */
    if (node->type != GUMBO_NODE_ELEMENT) {
        return;
    }
    GumboAttribute *href;
    if (node->v.element.tag == GUMBO_TAG_A &&
        (href = gumbo_get_attribute(&node->v.element.attributes, "href"))) {
        sds url = sdsnew(href->value);
        /*
         * If the link found isn't a valid url, try to resolve reference.
         */
        if (!is_valid_url(url)) {
            url_t *resolved = resolve_reference(url, url_parse->raw);
            url = sdsnew(resolved->raw);
            free_url(resolved);
        }
        /**
         * if resolved path is valid according to the standard and
         * it meet spec's requirement for most, add the resolved path to job queue
         */
        if (is_valid_url(url) && url_validation(url_parse->raw, url)) {
            add_to_queue(url, seen, job_queue);
        } else {
            sdsfree(url);
        }
    }
    GumboVector *children = &node->v.element.children;
    /**
     * Search children nodes
     */
    for (unsigned int i = 0; i < children->length; ++i) {
        add_to_job_queue(url_parse, (GumboNode *) (children->data[i]), job_queue, seen);
    }
}

/**
 * Extract location header
 * Return NULL if not presented
 * @param header_map
 * @return
 */
sds getLocation(sds_map_t *header_map) {
    sds *location = (sds *) map_get(header_map, LOCATION);
    if (location) {
        return *location;
    } else {
        return NULL;
    }
}

/**
 * Extract content location header
 * Return NULL if not presented
 * @param header_map
 * @return
 */
sds getContentLocation(sds_map_t *header_map) {
    sds *redirect_to = (sds *) map_get(header_map, CONTENT_LOCATION);
    if (redirect_to) {
        return *redirect_to;
    } else {
        return NULL;
    }
}

/**
 * Extract content length header
 * Return NULL if not presented
 * @param header_map
 * @return
 */
sds getContentLength(sds_map_t *header_map) {
    sds *content_length = (sds *) map_get(header_map, CONTENT_LENGTH);
    if (content_length) {
        return *content_length;
    } else {
        return NULL;
    }
}

/**
 * Extract content type header
 * Return NULL if not presented
 * @param header_map
 * @return
 */
sds getContentType(sds_map_t *header_map) {
    sds *type = (sds *) map_get(header_map, CONTENT_TYPE);
    if (type) {
        return *type;
    } else {
        return NULL;
    }
}


/**
 * Check if the full header has been received by looking for \r\n\r\n
 * Return NULL if not yet received
 * @param buffer
 * @return char* pointing to the head of the body
 */
char *locate_body(char *buffer) {
    char *header_body_separator = "\r\n\r\n";
    return strstr(buffer, header_body_separator);
}

/**
 * Extract headers from response as a map
 * @param buffer
 * @return sds_map_t
 */
sds_map_t *extract_header(char *buffer) {
    /* Search for the body */
    char *where_body_is = locate_body(buffer);
    if (!where_body_is) {
        return NULL;
    }
    /*
     * Determine the header length
     */
    int header_size = (int) strlen(buffer) - (int) strlen(where_body_is);
    sds header = sdscpylen(sdsempty(), buffer, header_size);
    sds *status_line;
    int header_count, field_count;

    /* Initialise a map */
    sds_map_t *map = malloc(sizeof(*map));
    map_init(map);

    /* Tokenize header into lines */
    sds *lines = sdssplitlen(header, sdslen(header), "\r\n", 2, &header_count);

    for (int j = 0; j < header_count; j++) {
        // Split status line
        if (j == 0) {
            status_line = sdssplitlen(lines[j], sdslen(lines[j]), " ", 1, &field_count);
            sdsfreesplitres(status_line, field_count);
        } else {
            // Locate the first colon
            char *first_colon = strstr(lines[j], ":");
            sds name = sdscpylen(sdsempty(), lines[j], strlen(lines[j]) - strlen(first_colon));
            sds value = sdsnew(first_colon + 1);
            sdstrim(value, " \n");

            // Add header to map
            map_set(map, lower(name), sdsdup(value));
            sdsfree(name);
            sdsfree(value);
        }
    }
    sdsfreesplitres(lines, header_count);
    sdsfree(header);
    return map;
}

/**
 * Determines if buffer is large enough to receive full response
 * @param header_map
 * @return
 */
bool isBufferSufficient(sds_map_t *header_map) {
    sds content_length = getContentLength(header_map);
    if (content_length) {

        int expected = 0;
        if (!parse_int(content_length, &expected)) {
            return false;
        }
        if (expected > RESPONSE_BUFFER - 1) {
            log_error("\t|- Aborted: Expected Content-Length exceeds buffer size %d > %d", expected, RESPONSE_BUFFER);
            return false;
        }
    }
    return true;
}

/**
 * A wrapper of strtol with proper logging
 * @param string
 * @param parse_result
 * @return
 */
bool parse_int(sds string, int *parse_result) {
    errno = 0;
    char *ptr = NULL;
    *parse_result = (int) strtol(string, &ptr, 10);
    if (errno) {
        log_error("Parsing integer failed");
        return false;
    }
    return true;
}

/**
 * Check if the MIME type is html
 * @param header_map
 * @return
 */
bool isHTML(sds_map_t *header_map) {
    sds type = getContentType(header_map);
    // if content type header is presented
    if (type) {
        if (!strstr(type, HTML_ONLY)) {
            log_error("\t|- Aborted: Content type is not HTML");
            return false;
        }
    }
    // otherwise return true
    return true;
}