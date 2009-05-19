// nscp_p.h --- 
// 
// Author: liuguangzhao
// Copyright (C) 2007-2010 liuguangzhao@users.sf.net
// URL: http://www.qtchina.net http://nullget.sourceforge.net
// Created: 2009-05-19 21:07:29 +0800
// Last-Updated: 2009-05-19 22:27:14 +0800
// Version: $Id$
// 

#ifndef _NSCP_P_H_
#define _NSCP_P_H_

#include "nullfxp_config.h"

#include <libssh2.h>

#ifdef HAVE_WINSOCK2_H
# include <winsock2.h>
#endif
#ifdef HAVE_NETINET_IN_H
# include <netinet/in.h>
#endif
#ifdef HAVE_SYS_SOCKET_H
# include <sys/socket.h>
#endif
# ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_ARPA_INET_H
# include <arpa/inet.h>
#endif
#ifdef HAVE_SYS_TIME_H
# include <sys/time.h>
#endif

#include <sys/types.h>
#include <fcntl.h>
#include <errno.h>
#include <stdio.h>
#include <ctype.h>

#include "libssh2.h"
#include "libssh2_sftp.h"

typedef struct ssh_conn
{
    LIBSSH2_SESSION *sess;
    LIBSSH2_SFTP *sftp;
    int sock;
} ssh_conn_t;

ssh_conn_t *connect_to_host(char *host, short port);
int  disconnect_from_host(ssh_conn_t *conn);

int connect_auth_password(ssh_conn_t *conn, char *user, char *password);
int scp_file_to_server_on_sftp(ssh_conn_t *conn, char *local_file, char *remote_file);

#endif /* _NSCP_P_H_ */
