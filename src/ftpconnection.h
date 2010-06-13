// ftpconnection.h --- 
// 
// Author: liuguangzhao
// Copyright (C) 2007-2010 liuguangzhao@users.sf.net
// URL: http://www.qtchina.net http://nullget.sourceforge.net
// Created: 2009-09-06 10:33:59 +0800
// Version: $Id$
// 

#ifndef _FTPCONNECTION_H_
#define _FTPCONNECTION_H_

#include "connection.h"

class LibFtp;

class FTPConnection : public Connection
{
    Q_OBJECT;
public:
    FTPConnection(QObject *parent = 0);
    ~FTPConnection();

    virtual int connect();
    virtual int disconnect();
    virtual int reconnect();
    virtual bool isConnected();
    virtual bool isRealConnected();

    virtual QTextCodec *codecForEnv(QString env);

    virtual QString getServerBanner();
    virtual QString getServerWelcome();

public slots:
    virtual int alivePing();

private:

};

#endif /* _FTPCONNECTION_H_ */
