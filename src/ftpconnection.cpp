// ftpconnection.cpp --- 
// 
// Author: liuguangzhao
// Copyright (C) 2007-2010 liuguangzhao@users.sf.net
// URL: http://www.qtchina.net http://nullget.sourceforge.net
// Created: 2009-09-06 10:35:47 +0800
// Version: $Id$
// 

#include "utils.h"

#include "ftpconnection.h"

FTPConnection::FTPConnection(QObject *parent)
    : Connection(parent)
{
}
FTPConnection::~FTPConnection()
{
    if (this->qsock != NULL) {
        delete this->qsock;
        this->qsock = NULL;
    }
    if (this->qdsock != NULL) {
        delete this->qdsock;
        this->qdsock = NULL;
    }
}

int FTPConnection::connect()
{
    this->qsock = new QTcpSocket();
    this->qsock->connectToHost(this->hostName, this->port);
    if (this->qsock->waitForConnected()) {
        q_debug()<<"ftp ctrl connect ok";
    } else {
        q_debug()<<this->qsock->errorString();
        return Connection::CONN_OTHER;
    }
    this->homePath = QString("/");
    return 0;
}
int FTPConnection::disconnect()
{
    return 0;
}
int FTPConnection::reconnect()
{
    return 0;
}
bool FTPConnection::isConnected()
{
    return Connection::isConnected();
}
bool FTPConnection::isRealConnected()
{
    return Connection::isRealConnected();
}

int FTPConnection::alivePing()
{
    return 0;
}

