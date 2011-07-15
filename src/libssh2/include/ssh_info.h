// info.h --- 
// 
// Author: liuguangzhao
// Copyright (C) 2007-2010 liuguangzhao@users.sf.net
// URL: http://www.qtchina.net http://nullget.sourceforge.net
// Created: 2009-07-19 11:20:20 +0000
// Version: $Id$
// 

#ifndef INFO_H
#define INFO_H 1

#ifdef __cplusplus
extern "C" {
#endif

#include <assert.h>

#include "libssh2.h"
#include "libssh2_sftp.h"



LIBSSH2_API char *libssh2_session_get_remote_version(LIBSSH2_SESSION *session);
LIBSSH2_API char **libssh2_session_get_remote_info(LIBSSH2_SESSION *session, char **info_vec);

LIBSSH2_API int libssh2_sftp_get_version(LIBSSH2_SFTP * sftp);

LIBSSH2_API LIBSSH2_SESSION *libssh2_session_for_sftp(LIBSSH2_SFTP *sftp);

LIBSSH2_API int libssh2_publickey_is_privatekey(const char *keypath, const char *passphrase);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* INFO_H */
