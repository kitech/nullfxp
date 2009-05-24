/* nscp_o.c --- 
 * 
 * Author: liuguangzhao
 * Copyright (C) 2007-2010 liuguangzhao@users.sf.net
 * URL: http://www.qtchina.net http://nullget.sourceforge.net
 * Created: 2009-05-24 17:23:35 +0800
 * Last-Updated: 2009-05-24 19:58:48 +0800
 * Version: $Id$
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <getopt.h>

#include "nscp_o.h"

void print_usage()
{
    fprintf(stderr, 
            "usage: nscp [-hup] [[user@]host1:]file1 ... [[user@]host2:]file2\n");
}

void nscp_option_set_default(nscp_option_t *nopt)
{
    assert(nopt != NULL);
    memset(nopt, 0, sizeof(nscp_option_t));
    nopt->port = 22;
}

void nscp_option_delete(nscp_option_t *nopt)
{
    assert(nopt != NULL);

    int i = 0;

    if (nopt->d_path != NULL) {
        nscp_path_delete(nopt->d_path);
    }
    
    for (i = 0; i < nopt->s_path_count; i ++) {
        if (nopt->s_paths[i] != NULL) {
            nscp_path_delete(nopt->s_paths[i]);
        }
    }
    if (nopt->s_paths != NULL) {
        free(nopt->s_paths);
    }
}

int  nscp_option_check_validation(nscp_option_t *nopt)
{
    assert(nopt != NULL);

    if (nopt->user_name == NULL) {
        fprintf(stderr, "Warning: usering current user '%s' instead.\n",
                getenv("USER"));
        nopt->user_name = getenv("USER:");
    }
    
    if (nopt->password == NULL) {
        fprintf(stderr, "Error: password can not be empty.\n");
        exit(1);
    }

    int iport = nopt->port;
    if (!(iport > 0 && iport <= 65535)) {
        fprintf(stderr, "Error: port can not be %d.\n", nopt->port);
        exit(1);
    }

    if ((nopt->direct != DIRECT_UPLOAD) 
        && (nopt->direct != DIRECT_DOWNLOAD)) {
        fprintf(stderr, "Error: destination path can not be empty.\n");
        exit(1);
    }

    return 0;
}

nscp_path_t *nscp_path_new(char *u, char *p, char *h, char *path)
{
    nscp_path_t *np = (nscp_path_t*)calloc(1, sizeof(nscp_path_t));
    
    np->user_name = u;
    np->password = p;
    np->host = h;
    np->path = path;

    if (np->host != NULL) {
        np->path_type = PATH_SSH;
    } else {
        np->path_type = PATH_FILE;
    }

    return np;
}

void nscp_path_delete(nscp_path_t *np)
{
    assert(np != NULL);
    // 注意这里的小技巧，这几个指针其实是指向同一块内存不位置的指针
    if (np->user_name != NULL) {
        free(np->user_name);
    } else if (np->password != NULL) {
        free(np->password);
    } else if (np->host != NULL) {
        free(np->host);
    } else if (np->path != NULL) {
        free(np->path);
    } else {
        assert(0);
    }
    free(np);
}

int  nscp_option_parse_path(int argc, char **argv, nscp_option_t *nopt)
{
    int idx = argc - 1;
    char *ptr = NULL;
    char *p = NULL;
    char *u = NULL;
    char *h = NULL;
    char *path = NULL;

    if (argc - nopt->none_path_count < 3) {
        fprintf(stderr, "source path or dest path can not be empty.\n");
        exit(1);
    }

    assert(argv[idx] != NULL);    
    // parse dest path
    ptr = strdup(argv[idx]);
    if ((p = strstr(ptr, ":/")) != NULL) {
        path = p + 1;
        *p = '\0';
        if ((p = strchr(ptr, '@')) != NULL) {
            h = p + 1;
            *p = '\0';
            u = ptr;
            nopt->user_name = strdup(u);
        } else {
            h = ptr;
        }
        nopt->direct = DIRECT_UPLOAD;
    } else {
        path = ptr;
        nopt->direct = DIRECT_DOWNLOAD;
    }

    fprintf(stderr, "Info: dest %s@%s:%s\n", u, h, path);
    nopt->d_path = nscp_path_new(u, NULL, h, path);

    nopt->s_path_count = idx - nopt->none_path_count - 1;
    fprintf(stderr, "Info: source file count: %d.(%d-%d-1)\n", nopt->s_path_count,
            idx, nopt->none_path_count);
    assert(nopt->s_path_count > 0);
    nopt->s_paths = (nscp_path_t**)calloc(nopt->s_path_count, sizeof(nscp_path_t*));
    int s_path_idx = nopt->s_path_count - 1;

    // parse source path
    while (-- idx > nopt->none_path_count) {
        p = h = u = path = NULL;
        ptr = strdup(argv[idx]);        
        fprintf(stderr, "Info: source, %s\n", ptr);

        if ((p = strstr(ptr, ":/")) != NULL) {
            if (nopt->direct == DIRECT_UPLOAD) {
                fprintf(stderr, "Error: can not nscp from a ssh file to anothor ssh file.\n");
                exit(1);
            }
            path = p + 1;
            *p = '\0';
            if ((p = strchr(ptr, '@')) != NULL) {
                h = p + 1;
                *p = '\0';
                u = ptr;
            } else {
                h = ptr;
            }
        } else {
            if (nopt->direct == DIRECT_DOWNLOAD) {
                fprintf(stderr, "Error: can not nscp '%s' to another local file, using 'cp' instead.\n",
                        ptr);
                exit(1);
            }
            path = ptr;
        }
        fprintf(stderr, "Info: dest %s@%s:%s\n", u, h, path);
        nopt->s_paths[s_path_idx --] = nscp_path_new(u, NULL, h, path);
    }

    return 0;
}
