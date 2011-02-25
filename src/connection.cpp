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
    this->user_canceled = false;
    this->codec = NULL;
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

bool Connection::setUserCanceled()
{
    this->user_canceled = true;
    return true;
}

void Connection::setHostInfo(QMap<QString, QString> host)
{
    this->protocol = host["protocol"];
    this->userName = host["user_name"];
    this->password = host["password"];
    this->hostName = host["host_name"];
    this->port = host["port"].toUShort();
    if (host.contains("pubkey")) {
        this->pubkey = host["pubkey"];
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

QString Connection::get_status_desc(int status)
{
    static const char *status_desc[] = {
        "CONN_OK",
        "CONN_REFUSE",
        "CONN_CANCEL",
        "CONN_OTHER",
        "CONN_RESOLVE_ERROR",
        "CONN_SESS_ERROR",
        "CONN_AUTH_ERROR",
        "CONN_SFTP_ERROR",
        "CONN_EXEC_ERROR"
    };

    QString emsg = QString(tr("No error."));
    switch (status) {
    case Connection::CONN_REFUSE:
        emsg = QString(tr("Remote host not usable."));
        break;
    case Connection::CONN_AUTH_ERROR:
        emsg = QString(tr("Auth faild. Check your name and password and retry again."));
        break;
    case Connection::CONN_RESOLVE_ERROR:
        emsg = QString(tr("Can not resolve host name."));
        break;
    case Connection::CONN_SESS_ERROR:
        emsg = QString(tr("Can not initial SSH session."));
        break;
    case Connection::CONN_SFTP_ERROR:
        emsg = QString(tr("Can not initial SFTP handle."));
        break;
    case Connection::CONN_PROTOCOL_VERSION_NOT_MATCH_ERROR:
        emsg = QString(tr("Server protocol version not match. Is it 1.x ?"));
        break;
    default:
        emsg = QString(tr("Unknown error."));
        break;
    }
    
    if ((unsigned int)status > sizeof(status_desc)/sizeof(char*)) {
        return "Unknown status";
    } else {
        return emsg;
    }
}

QTextCodec *Connection::codecForEnv(QString env)
{
    return NULL;
}
QTextCodec *Connection::codecForConnect()
{
    return this->codec;
}

QString Connection::errorString()
{
    return this->mErrorString;
    return QString();
}
