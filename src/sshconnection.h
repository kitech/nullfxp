// sshconnection.h --- 
// 
// Author: liuguangzhao
// Copyright (C) 2007-2010 liuguangzhao@users.sf.net
// URL: http://www.qtchina.net http://nullget.sourceforge.net
// Created: 2009-09-06 10:33:59 +0800
// Version: $Id$
// 

#ifndef _SSHCONNECTION_H_
#define _SSHCONNECTION_H_

#include "connection.h"

class SSHConnection : public Connection
{
    Q_OBJECT;
public:
    SSHConnection(QObject *parent = 0);
    virtual ~SSHConnection();

    virtual int connect();
    virtual int disconnect();
    virtual int reconnect();
    // virtual bool isConnected();
    // virtual bool isRealConnected();

    virtual QTextCodec *codecForEnv(const QString &env);

    // libssh2 callbacks, prototype in libssh2.h:236
    static void callback_debug_wrapper(LIBSSH2_SESSION *session, int always_display, const char *message, 
                                       int message_len, const char *language, int language_len, 
                                       void **abstract);
    void backend_debug(LIBSSH2_SESSION *session, int always_display, const char *message, 
                       int message_len, const char *language, int language_len, 
                       void **abstract);

    static void callback_disconnect_wrapper(LIBSSH2_SESSION *session, int reason, const char *message, 
                                      int message_len, const char *language, int language_len, 
                                      void **abstract);

    void backend_disconnect(LIBSSH2_SESSION *session, int reason, const char *message, 
                                      int message_len, const char *language, int language_len, 
                                      void **abstract);

public slots:
    virtual int alivePing();
    int listSessionMethords();

private:
    int initSocket();
    void piClose(int sock);
    int initSSHSession();
    int sshAuth();
    int sshHomePath();
    QString get_server_env_vars(const char *cmd);
    QString libssh2SessionLastErrorString();
    int setCallbacks();

private:
    static int gLibssh2Inited;
    static QAtomicInt gLibssh2UseCount;
};

#endif /* _SSHCONNECTION_H_ */
