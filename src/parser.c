//
// Created by Haswe on 3/25/2020.
//
#include "parser.h"
#include "response.h"

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


