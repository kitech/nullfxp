// connection.h --- 
// 
// Author: liuguangzhao
// Copyright (C) 2007-2010 liuguangzhao@users.sf.net
// URL: http://www.qtchina.net http://nullget.sourceforge.net
// Created: 2009-05-28 23:13:36 +0000
// Last-Updated: 
// Version: $Id$
// 

#ifndef _CONNECTION_H_
#define _CONNECTION_H_

#include <QtCore>
#include <QThread>

#include "libssh2.h"
#include "libssh2_sftp.h"

class Connection : public QObject
{
    Q_OBJECT;
public:
    Connection(QObject *parent = 0);
    ~Connection();

    virtual int connect();
    virtual int disconnect();
    virtual int reconnect();
    
public slots:
    virtual int alivePing();

public:
    QString userName;
    QString password;   //存储的密码为url编码过的
    QString decodedPassword;
    QString hostName ;
    short   port;
    QString pubkeyPath;
    QString homePath;
    
    LIBSSH2_SESSION *sess;
    int sock;

signals:
    void alivePong(int alive);

};
        
        
#endif /* _CONNECTION_H_ */
