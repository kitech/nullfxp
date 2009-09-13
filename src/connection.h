// connection.h --- 
// 
// Author: liuguangzhao
// Copyright (C) 2007-2010 liuguangzhao@users.sf.net
// URL: http://www.qtchina.net http://nullget.sourceforge.net
// Created: 2009-05-28 23:13:36 +0000
// Version: $Id$
// 

#ifndef _CONNECTION_H_
#define _CONNECTION_H_

#include <QtCore>
#include <QThread>
#include <QtNetwork>

#include "libssh2.h"
#include "libssh2_sftp.h"

class LibFtp;

// 这个类及其子类的操作都是阻塞的socket
class Connection : public QObject
{
    Q_OBJECT;
public:
    enum {CONN_OK=0,CONN_REFUSE,CONN_CANCEL,CONN_OTHER,CONN_RESOLVE_ERROR,
          CONN_SESS_ERROR,CONN_AUTH_ERROR,CONN_SFTP_ERROR,CONN_EXEC_ERROR,
          CONN_PROTOCOL_VERSION_NOT_MATCH_ERROR};
    enum {PROTO_MIN = 0, 
          PROTO_SFTP, PROTO_FTP, PROTO_FTPS,
          PROTO_MAX};

    Connection(QObject *parent = 0);
    ~Connection();

    virtual int connect();
    virtual int disconnect();
    virtual int reconnect();
    virtual bool isConnected();
    virtual bool isRealConnected();
    void setHostInfo(QMap<QString, QString> host);
    QMap<QString, QString> hostInfo();
    QString userHomePath();
    int protocolType();

    virtual QString get_status_desc(int status);

public slots:
    virtual int alivePing();

public:
    bool connected; 
    QMap<QString, QString> mHostInfo;
    QString protocol;
    QString userName;
    QString password;   //存储的密码为url编码过的
    QString decodedPassword; // 没用了？
    QString hostName;
    short   port;
    QString pubkey;
    QString homePath;
    
    LIBSSH2_SESSION *sess;
    int sock;
    int dsock; //data sock, if has
    QTcpSocket *qsock;  // 将int sock放到这个对象中，因为需要兼容不同的协议，使用不同的socket对象
    QTcpSocket *qdsock;

    bool user_canceled;

    LibFtp *ftp;
signals:
    void alivePong(int alive);
    void connect_state_changed(QString state_desc);
};


#endif /* _CONNECTION_H_ */
