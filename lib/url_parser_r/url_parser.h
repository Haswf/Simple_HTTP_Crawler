//
// Created by Haswe on 3/25/2020.
//

/*_
 * Copyright 2010 Scyphus Solutions Co. Ltd.  All rights reserved.
 *
 * Authors:
 *      Hirochika Asai
 */

#ifndef _URL_PARSER_H
#define _URL_PARSER_H

#include "../sds/sds.h"

/*
 * URL storage
 */
typedef struct parsed_url {
    char *origin;
    char *scheme;               /* mandatory */
    char *host;                 /* mandatory */
    char *port;                 /* optional */
    char *path;                 /* optional */
    char *query;                /* optional */
    char *fragment;             /* optional */
    char *username;             /* optional */
    char *password;             /* optional */
} parsed_url_t;

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Declaration of function prototypes
 */
parsed_url_t *parse_url(char *);

void parsed_url_free(parsed_url_t *);

#ifdef __cplusplus
}
#endif

#endif /* _URL_PARSER_H */

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: sw=4 ts=4 fdm=marker
 * vim<600: sw=4 ts=4
 */