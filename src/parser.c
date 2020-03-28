//
// Created by Haswe on 3/25/2020.
//
#include "parser.h"
#include "../lib/log/log.h"
#include "crawler.h"

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

void add_to_job_queue(parsed_url_t *url_parse, GumboNode *node, sds_vec_t *job_queue, int_map_t *seen) {
    if (node->type != GUMBO_NODE_ELEMENT) {
        return;
    }
    GumboAttribute *href;
    if (node->v.element.tag == GUMBO_TAG_A &&
        (href = gumbo_get_attribute(&node->v.element.attributes, "href"))) {
        sds url = sdsnew(href->value);
        if (!is_valid_url(url)) {
            url = resolve_referencing(url, url_parse->origin);
        }
        if (url && url_validation(url_parse->origin, url)) {
            add_absolute_to_queue(url, seen, job_queue);
        }
    }
    GumboVector *children = &node->v.element.children;
    for (unsigned int i = 0; i < children->length; ++i) {
        add_to_job_queue(url_parse, (GumboNode *) (children->data[i]), job_queue, seen);
    }
}

void search_and_add_url(parsed_url_t *url_parse, sds html, sds_vec_t *job_queue, int_map_t *seen) {
    GumboOutput *output = gumbo_parse(html);
    add_to_job_queue(url_parse, output->root, job_queue, seen);
    gumbo_destroy_output(&kGumboDefaultOptions, output);
}

sds resolve_referencing(sds relative_path, sds base_url) {
    UriUriA current;
    UriUriA relative;
    UriUriA resolved;
    const char *errorPos;
    if (uriParseSingleUriA(&current, base_url, &errorPos) != URI_SUCCESS) {
        return NULL;
    }

    if (!strstr(relative_path, ".") && !strstr(relative_path, "/")) {
        relative_path = sdscatprintf(sdsempty(), "./%s", relative_path);
    }

    if (uriParseSingleUriA(&relative, relative_path, &errorPos) != URI_SUCCESS) {
        return NULL;
    }

    if (uriAddBaseUriA(&resolved, &relative, &current) != URI_SUCCESS) {
        /* Failure */
        return NULL;
    }

    char *uriString;
    int charsRequired = 0;
    if (uriToStringCharsRequiredA(&resolved, &charsRequired) != URI_SUCCESS) {
        return NULL;
    }
    charsRequired++;
    uriString = malloc(charsRequired * sizeof(char));
    if (uriString == NULL) {
        /* Failure */
        return NULL;
    }
    if (uriToStringA(uriString, &resolved, charsRequired, NULL) != URI_SUCCESS) {
        return NULL;
    }
    uriFreeUriMembersA(&relative);
    uriFreeUriMembersA(&current);
    uriFreeUriMembersA(&resolved);

    return sdsnew(uriString);
}

