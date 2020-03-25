//
// Created by Haswe on 3/25/2020.
//
#include "parser.h"


static void search_for_links(GumboNode *node) {

    if (node->type != GUMBO_NODE_ELEMENT) {
        return;
    }
    GumboAttribute *href;
    if (node->v.element.tag == GUMBO_TAG_A &&
        (href = gumbo_get_attribute(&node->v.element.attributes, "href"))) {
        printf("%s\n", href->value);
    }

    GumboVector *children = &node->v.element.children;
    for (unsigned int i = 0; i < children->length; ++i) {
        search_for_links((GumboNode *) (children->data[i]));
    }
}

void print_url(Response *response) {
    GumboOutput *output = gumbo_parse(response->body);
    search_for_links(output->root);
    gumbo_destroy_output(&kGumboDefaultOptions, output);
}

void add_to_job_queue(GumboNode *node, sds_vec_t *job_queue, int_map_t *seen) {
    if (node->type != GUMBO_NODE_ELEMENT) {
        return;
    }
    GumboAttribute *href;
    if (node->v.element.tag == GUMBO_TAG_A &&
        (href = gumbo_get_attribute(&node->v.element.attributes, "href"))) {
        if (map_get(seen, href->value) == NULL) {
            vec_push(job_queue, sdsnew(href->value));

        }
//        printf("%s %d\n", href->value, job_queue->length);
    }

    GumboVector *children = &node->v.element.children;
    for (unsigned int i = 0; i < children->length; ++i) {
        add_to_job_queue((GumboNode *) (children->data[i]), job_queue, seen);
    }
}

void add_url(Response *response, sds_vec_t *job_queue, int_map_t *seen) {
    GumboOutput *output = gumbo_parse(response->body);
    add_to_job_queue(output->root, job_queue, seen);
    gumbo_destroy_output(&kGumboDefaultOptions, output);
}

