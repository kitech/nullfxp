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
    virtual bool isConnected();
    virtual bool isRealConnected();

    virtual QTextCodec *codecForEnv(QString env);

public slots:
    virtual int alivePing();

private:
    int initSocket();
    void piClose(int sock);
    int initSSHSession();
    int sshAuth();
    int sshHomePath();
    QString get_server_env_vars(const char *cmd);
    QString libssh2SessionLastErrorString();
};

#endif /* _SSHCONNECTION_H_ */
