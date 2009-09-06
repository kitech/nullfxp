// connection.cpp --- 
// 
// Author: liuguangzhao
// Copyright (C) 2007-2010 liuguangzhao@users.sf.net
// URL: http://www.qtchina.net http://nullget.sourceforge.net
// Created: 2009-05-28 23:15:47 +0000
// Version: $Id$
// 

#include "connection.h"

Connection::Connection(QObject *parent)
    : QObject(parent), connected(false)
{
    this->sock = -1;
    this->dsock = -1;
    this->qsock = NULL;
    this->qdsock = NULL;
}
Connection::~Connection()
{
}

int Connection::alivePing()
{
    return 0;
}

int Connection::connect()
{
    return 0;
}
int Connection::disconnect()
{
    return 0;
}
int Connection::reconnect()
{
    return 0;
}
bool Connection::isConnected()
{
    return this->connected;
}
bool Connection::isRealConnected()
{
    return this->connected;
}

void Connection::setHostInfo(QMap<QString, QString> host)
{
    this->protocol = host["protocol"];
    this->userName = host["user_name"];
    this->password = host["password"];
    this->hostName = host["host_name"];
    this->port = host["port"].toShort();
    if (host.contains("pubkey")) {
        this->pubkeyPath = host["pubkey"];
    }
    this->mHostInfo = host;
}

QMap<QString, QString> Connection::hostInfo()
{
    return this->mHostInfo;
}

QString Connection::userHomePath()
{
    return this->homePath;
}

int Connection::protocolType()
{
    if (this->protocol == "FTPS") {
        return PROTO_FTPS;
    } else if (this->protocol == "FTP") {
        return PROTO_FTP;
    } else if (this->protocol == "SFTP") {
        return PROTO_SFTP;
    } else {
    }
    return PROTO_MIN;
}
