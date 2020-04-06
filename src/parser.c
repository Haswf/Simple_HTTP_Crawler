/**
 * Module to parse html with Gumbo
 * Created by Shuyang Fan on 3/25/2020.
 */

#include "config.h"
#include "parser.h"
#include "../lib/log/log.h"
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
 * Helper function to add
 * @param url_parse
 * @param node
 * @param job_queue
 * @param seen
 */
void add_to_job_queue(url_t *url_parse, GumboNode *node, sds_vec_t *job_queue, int_map_t *seen) {
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

