//
// Created by Haswe on 3/31/2020.
//

#include "url.h"
#include <regex.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#define SCHEME_INDEX 2
#define AUTHORITY_INDEX 4
#define PATH_INDEX 5
#define QUERY_INDEX 7
#define FRAGMENT_INDEX 8


/**
 * Interprets and removes the special "." and ".." complete path segments from a referenced path.
 * This implementation is based on pseudocode at https://tools.ietf.org/html/rfc3986#section-5.1
 * @param input
 * @return
 */
sds remove_dot_segment(sds input) {
    /*
     * The input buffer is initialized with the now-appended path
       components and the output buffer is initialized to the empty
       string.
     */
    sds output = sdsempty();

    // While the input buffer is not empty, loop as follows:
    while (sdslen(input) > 0) {
        char *key = NULL;
        /* If the input buffer begins with a prefix of "../" or "./",
           then remove that prefix from the input buffer; otherwise,
           */
        if (strstr(input, key = "../") == input || strstr(input, key = "./") == input) {
            sdsrange(input, strlen(key), sdslen(input));
            continue;
        }
        /*
           if the input buffer begins with a prefix of "/./" or "/.",
           where "." is a complete path segment, then replace that
           prefix with "/" in the input buffer; otherwise,
        */
        if (strstr(input, key = "/./") == input ||
            (strstr(input, key = "/.") == input && strstr(input, "/..") != input)) {
            sdsrange(input, strlen(key), sdslen(input));
            input = sdscatprintf(sdsempty(), "/%s", input);
            continue;
        }

        /*
         * C.  if the input buffer begins with a prefix of "/../" or "/..",
           where ".." is a complete path segment, then replace that
           prefix with "/" in the input buffer and remove the last
           segment and its preceding "/" (if any) from the output
           buffer; otherwise
         */
        if (strstr(input, key = "/../") == input || strstr(input, key = "/..") == input) {
            sdsrange(input, strlen(key), sdslen(input));
            input = sdscatprintf(sdsempty(), "/%s", input);
            // If the right most slash is at the start, i.e. output buffer only has one path segment
            if (sdslen(output)) {
                char *preceding_slash = strrchr(output, '/');
                if (preceding_slash == output) {
                    sdsclear(output);
                } else {
                    sdsrange(output, 0, sdslen(output) - strlen(preceding_slash) - 1);
                }
                continue;
            }

        }

        /*
         * if the input buffer consists only of "." or "..", then remove
           that from the input buffer; otherwise,
         */
        sdstrim(input, ".");


        /*
         * move the first path segment in the input buffer to the end of
           the output buffer, including the initial "/" character (if
           any) and any subsequent characters up to, but not including,
           the next "/" character or the end of the input buffer.
         */
        char *slash_pos = strstr(input, "/");
        // Move the initial "/" character if presented
        if (slash_pos == input) {
            // move the slash to the output first
            output = sdscatlen(output, input, 1);
            sdsrange(input, 1, sdslen(input));
        };

        // Locates the next slash character
        slash_pos = strstr(input, "/");
        int copy_size = 0;
        // Move the path between these two slashes
        if (slash_pos) {
            copy_size = (int) sdslen(input) - (int) strlen(slash_pos);
        }
            // Move the characters up to the end of the input buffer.
        else {
            copy_size = sdslen(input);
        }
        output = sdscatlen(output, input, copy_size);
        sdsrange(input, copy_size, sdslen(input));

    }
    return output;
}

/**
 * Merges a relative path with the path of base URI.
 * This implementation is based on pseudocode at https://tools.ietf.org/html/rfc3986#section-5.1
 * @param base_uri
 * @param relative_path
 * @return
 */
sds merge_path(sds base_uri, sds relative_path) {
    url_t *parsed = parse_url(base_uri);
    /*
     * If the base URI has a defined authority component and an empty
      path, then return a string consisting of "/" concatenated with the
      reference's path; otherwise,
     */
    if (parsed->authority && !parsed->path) {
        return sdscatprintf(sdsempty(), "/%s", relative_path);
    }

        /*
         * return a string consisting of the reference's path component
          appended to all but the last segment of the base URI's path (i.e.,
          excluding any characters after the right-most "/" in the base URI
          path, or excluding the entire base URI path if it does not contain
          any "/" characters).
         */

    else {
        char *right_most_slash = strrchr(parsed->path, '/');
        if (right_most_slash) {
            sds base_path = sdscatlen(sdsempty(), parsed->path, sdslen(parsed->path) - strlen(right_most_slash) + 1);
            return sdscat(base_path, relative_path);
        } else {
            return relative_path;
        }
    }

}

url_t *parse_url(sds text) {
    char pattern[80] = "^(([^:/?#]+):)?(//([^/?#]*))?([^?#]*)(\\?([^#]*))?(#(.*))?";
    regex_t compiled;
    regcomp(&compiled, pattern, REG_EXTENDED);

    int nsub = compiled.re_nsub;
    regmatch_t matchptr[nsub];
    int err = regexec(&compiled, text, nsub, matchptr, 0);
    if (err) {
        if (err == REG_NOMATCH) {
            printf("Regular expression did not match.\n");
        } else if (err == REG_ESPACE) {
            printf("Ran out of memory.\n");
        }
    };

    url_t *parsed = (url_t *) malloc(sizeof(*parsed));
    parsed->scheme = NULL;
    parsed->authority = NULL;
    parsed->path = NULL;
    parsed->query = NULL;
    parsed->fragment = NULL;
    parsed->raw = NULL;

    if (matchptr[SCHEME_INDEX].rm_so != -1) {
        sds copy = sdsnew(text);
        sdsrange(copy, matchptr[SCHEME_INDEX].rm_so, matchptr[SCHEME_INDEX].rm_eo - 1);
        parsed->scheme = copy;
    }

    if (matchptr[AUTHORITY_INDEX].rm_so != -1) {
        sds copy = sdsnew(text);
        sdsrange(copy, matchptr[AUTHORITY_INDEX].rm_so, matchptr[AUTHORITY_INDEX].rm_eo - 1);
        parsed->authority = copy;
    }

    if (matchptr[PATH_INDEX].rm_so != -1 && matchptr[PATH_INDEX].rm_so != matchptr[PATH_INDEX].rm_eo) {
        sds copy = sdsnew(text);
        sdsrange(copy, matchptr[PATH_INDEX].rm_so, matchptr[PATH_INDEX].rm_eo - 1);
        parsed->path = copy;
    }

    if (matchptr[QUERY_INDEX].rm_so != -1) {
        sds copy = sdsnew(text);
        sdsrange(copy, matchptr[QUERY_INDEX].rm_so, matchptr[QUERY_INDEX].rm_eo - 1);
        parsed->query = copy;
    }

    if (matchptr[FRAGMENT_INDEX].rm_so != -1) {
        sds copy = sdsnew(text);
        sdsrange(copy, matchptr[FRAGMENT_INDEX].rm_so, matchptr[FRAGMENT_INDEX].rm_eo - 1);
        parsed->fragment = copy;
    }
    parsed->raw = sdsnew(text);
    regfree(&compiled);
    return parsed;
}

int free_url(url_t *url) {
    if (url->scheme) {
        sdsfree(url->scheme);
    }
    if (url->authority) {
        sdsfree(url->authority);
    }
    if (url->path) {
        sdsfree(url->path);
    }
    if (url->query) {
        sdsfree(url->query);
    }
    if (url->fragment) {
        sdsfree(url->fragment);
    }
    if (url->raw) {
        sdsfree(url->raw);
    }
    free(url);
}

/**
 * Check if a url looks valid by attempting to parse it
 * @param url
 * @return 0 for doesn't look an url, 1 for looks like an url
 */
bool is_valid_url(sds url) {
    char pattern[80] = "^(([^:/?#]+):)?(//([^/?#]*))?([^?#]*)(\\?([^#]*))?(#(.*))?";
    regex_t compiled;
    regcomp(&compiled, pattern, REG_EXTENDED);

    int nsub = compiled.re_nsub;
    regmatch_t matchptr[nsub];
    int err = regexec(&compiled, url, nsub, matchptr, 0);
    if (err) {
        if (err == REG_NOMATCH) {
            printf("Regular expression did not match.\n");
        } else if (err == REG_ESPACE) {
            printf("Ran out of memory.\n");
        }
    }

    if (matchptr[SCHEME_INDEX].rm_so == -1 && matchptr[AUTHORITY_INDEX].rm_so == -1) {
        return false;
    }
    return true;
}

/**
 * Recompose an url from parsed url components.
 * This implementation follows the pseudocode at https://tools.ietf.org/html/rfc3986#section-5.3
 * @param url
 * @return
 */
sds recomposition(url_t *url) {
    sds result = sdsempty();
    if (url->scheme) {
        result = sdscatfmt(result, "%s:", url->scheme);
    }

    if (url->authority) {
        result = sdscatfmt(result, "//%s", url->authority);
    }
    result = sdscat(result, url->path);

    if (url->query) {
        result = sdscatfmt(result, "?%s", url->query);
    }

    if (url->fragment) {
        result = sdscatfmt(result, "#%s", url->fragment);
    }
    return result;
}

url_t *resolve_reference(sds reference, sds base) {
    url_t *reference_parsed = parse_url(reference);
    url_t *base_parsed = parse_url(base);
    url_t *target = malloc(sizeof(*target));

    if (reference_parsed->scheme) {
        target->scheme = reference_parsed->scheme;
        target->authority = reference_parsed->authority;
        target->path = remove_dot_segment(reference_parsed->path);
        target->query = reference_parsed->query;
    } else {
        if (reference_parsed->authority) {
            target->authority = reference_parsed->authority;
            target->path = remove_dot_segment(reference_parsed->path);
            target->query = reference_parsed->query;
        } else {
            if (!reference_parsed->path) {
                target->path = base_parsed->path;
                if (reference_parsed->query) {
                    target->query = reference_parsed->query;
                } else {
                    target->query = base_parsed->query;
                }
            } else {
                if (strstr(reference_parsed->path, "/") == reference_parsed->path) {
                    target->path = remove_dot_segment(reference_parsed->path);
                } else {
                    target->path = merge_path(base_parsed->path, reference_parsed->path);
                    target->path = remove_dot_segment(target->path);
                }
                target->query = reference_parsed->query;
            }
            target->authority = base_parsed->authority;
        }
        target->scheme = base_parsed->scheme;
    }
    target->fragment = reference_parsed->fragment;
    target->raw = recomposition(target);
    return target;
}

