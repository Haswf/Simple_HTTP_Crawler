//
// Created by Haswe on 3/25/2020.
//
#include "parser.h"
#include "response.h"
#include "../lib/uriparser/Uri.h"
#include "../lib/uriparser/UriBase.h"
#include "../lib/uriparser/UriBase.h"
#include "../lib/uriparser/UriDefsAnsi.h"
#include "../lib/uriparser/UriDefsConfig.h"
#include "../lib/uriparser/UriDefsUnicode.h"
#include "../lib/uriparser/UriIp4.h"


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

void resolve_relative_path() {
    UriUriA absoluteDest;
    UriUriA relativeSource;
    UriUriA absoluteBase;
    sds uri = sdsempty();

    const char *errorPos;

    if (uriParseSingleUriA(&absoluteBase, "http://www.google.com", &errorPos) != URI_SUCCESS) {
//        /* Failure (no need to call uriFreeUriMembersA) */return ...;
    }

    if (uriParseSingleUriA(&relativeSource, "./test.html", &errorPos) != URI_SUCCESS) {
//        /* Failure (no need to call uriFreeUriMembersA) */return ...;
    }


    /* relativeSource holds "../TWO" now */
    /* absoluteBase holds "file:///one/two/three" now */
    if (uriAddBaseUriA(&absoluteDest, &relativeSource, &absoluteBase) != URI_SUCCESS) {
        /* Failure */
        uriFreeUriMembersA(&absoluteDest);
    }

    int charsRequired;

    if (uriToStringCharsRequiredA(&absoluteDest, &charsRequired) != URI_SUCCESS) {
        /* Failure */
    }
    charsRequired++;
    sds uriString = sdsempty();
    if (uriToStringA(uriString, &absoluteDest, charsRequired, NULL) != URI_SUCCESS) {

    }


    /* absoluteDest holds "file:///one/TWO" now */
    printf("Resolved path: %s\n", uriString);
    uriFreeUriMembersA(&absoluteDest);
}