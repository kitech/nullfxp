// nscp_o.h --- 
// 
// Author: liuguangzhao
// Copyright (C) 2007-2010 liuguangzhao@users.sf.net
// URL: http://www.qtchina.net http://nullget.sourceforge.net
// Created: 2009-05-24 17:23:51 +0800
// Version: $Id$
// 

#ifndef _NSCP_O_H_
#define _NSCP_O_H_

typedef struct nscp_path {
    char *user_name;
    char *password;
    char *host;
    int port;
    char *path;
    int path_type; // FILE, SSH
} nscp_path_t;

typedef struct nscp_option
{
    int debug_level; 
    int resume;      // if support resume when the same file found
    int recursive;   // if support recursive scp when directory found
    char *user_name;
    char *password;
    char *host;
    int  port;
    int  direct; // s->c, c->s
    int  none_path_count;
    nscp_path_t *d_path;
    nscp_path_t **s_paths;
    int s_path_count;
} nscp_option_t;

// resolve scp direct from user's option
enum {DIRECT_UNKNOWN = 0, DIRECT_UPLOAD, DIRECT_DOWNLOAD};

// path type, source path, or dest path
enum {PATH_UNKNOWN = 0, PATH_FILE, PATH_SSH};

// debug level, the bigger, the more debug output
enum {DEBUG_LEVEL_NONE = 0, DEBUG_LEVEL_DEBUG, DEBUG_LEVEL_INFO, DEBUG_LEVEL_All};

void nscp_option_delete(nscp_option_t *nopt);
nscp_path_t *nscp_path_new(char *u, char *p, char *h, char *path);
void nscp_path_delete(nscp_path_t *np);

void print_usage();
void nscp_option_set_default(nscp_option_t *nopt);
int  nscp_option_check_validation(nscp_option_t *nopt);
int  nscp_option_parse_path(int argc, char **argv, nscp_option_t *nopt);

#endif /* _NSCP_O_H_ */
