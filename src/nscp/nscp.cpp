// nscp.cpp --- 
// 
// Author: liuguangzhao
// Copyright (C) 2007-2010 liuguangzhao@users.sf.net
// URL: http://www.qtchina.net http://nullget.sourceforge.net
// Created: 2009-05-19 20:50:20 +0800
// Last-Updated: 2009-05-19 22:27:21 +0800
// Version: $Id$
// 

#include <cstdlib>
#include <cstdio>
#include <cassert>

#include "nscp_p.h"

int main(int argc, char **argv)
{
    char *username;
    char *password;
    char *host;
    short port;
    ssh_conn_t *conn = NULL;
    int rc = 0;

    assert(argc == argc);
    assert(argv == argv);

    username = strdup("kitsoft");
    password = strdup("2113");
    host = strdup("127.0.0.1");
    port = 22;

    conn = connect_to_host(host, port);
    if (!conn) {
        fprintf(stderr, "Connect to host error.\n");
        exit(1);
    }
    
    rc = connect_auth_password(conn, username, password);
    if (rc != 0) {
        disconnect_from_host(conn);
        exit(2);
    }
    
    rc = scp_file_to_server_on_sftp(conn, "/home/gzleo/download/goalbit-0.4.2.tar.bz2",
                                    "/home/kitsoft/goalbit-0.4.2.tar.bz2");
    

    return 0;
}
