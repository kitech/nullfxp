// nscp.cpp --- 
// 
// Author: liuguangzhao
// Copyright (C) 2007-2010 liuguangzhao@users.sf.net
// URL: http://www.qtchina.net http://nullget.sourceforge.net
// Created: 2009-05-19 20:50:20 +0800
// Version: $Id$
// 

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <getopt.h>

#include "nscp_p.h"
#include "nscp_o.h"

int main(int argc, char **argv)
{
    char *host;
    ssh_conn_t *conn = NULL;
    int rc = 0;
    int opt;
    nscp_option_t nopt;

    nscp_option_set_default(&nopt);
    while ((opt = getopt(argc, argv, "hp:P:u:")) != -1) {
        switch (opt) {
        case 'h':
            print_usage();
            exit(1);
            break;
        case 'u':
            nopt.none_path_count += 2;
            nopt.user_name = optarg;
            break;
        case 'p':
            nopt.none_path_count += 2;
            fprintf(stderr, "password: %s\n", optarg);
            nopt.password = optarg;
            break;
        case 'P':
            nopt.none_path_count += 2;
            fprintf(stderr, "port: %s\n", optarg);
            nopt.port = atoi(optarg);
            break;
        default:
            break;
        }
    }
    nscp_option_parse_path(argc, argv, &nopt);
    nscp_option_check_validation(&nopt);

    // 
    host = nopt.d_path->host != NULL ? nopt.d_path->host : nopt.s_paths[0]->host;

    conn = connect_to_host(host, nopt.port);
    if (!conn) {
        fprintf(stderr, "Connect to host error.\n");
        nscp_option_delete(&nopt);
        exit(1);
    }
    
    rc = connect_auth_password(conn, nopt.user_name, nopt.password);
    if (rc != 0) {
        disconnect_from_host(conn);
        nscp_option_delete(&nopt);
        exit(2);
    }

    rc = do_scp(conn, &nopt);
    
     /* rc = scp_file_to_server_on_sftp(conn, "/home/gzleo/download/goalbit-0.4.2.tar.bz2", */
     /*                                 "/home/kitsoft/goalbit-0.4.2.tar.bz2"); */

    // rc = scp_file_to_server_on_scp(conn, "/home/gzleo/download/goalbit-0.4.2.tar.bz2",
    //                                "/home/kitsoft/goalbit-0.4.2.tar.bz2");
    
    disconnect_from_host(conn);
    nscp_option_delete(&nopt);

    return 0;
}

