// ftpconnection.cpp --- 
// 
// Author: liuguangzhao
// Copyright (C) 2007-2010 liuguangzhao@users.sf.net
// URL: http://www.qtchina.net http://nullget.sourceforge.net
// Created: 2009-09-06 10:35:47 +0800
// Version: $Id$
// 

#include "utils.h"

#include "libftp/libftp.h"
#include "ftpconnection.h"

FTPConnection::FTPConnection(QObject *parent)
    : Connection(parent)
{
}
FTPConnection::~FTPConnection()
{
    if (this->ftp != NULL) {
        delete this->ftp;
    }
}

int FTPConnection::connect()
{
    this->ftp = new LibFtp();
    emit connect_state_changed(tr("Connecting to %1:%2 ...")
                               .arg(this->hostName).arg(this->port));
    if (this->ftp->connect(this->hostName, this->port) != 0) {
        return Connection::CONN_OTHER;
    }
    emit connect_state_changed(tr("Connect OK %1:%2. Login %3@%4 ...")
                               .arg(this->hostName).arg(this->port)
                               .arg(this->userName).arg(this->hostName));
    if (this->ftp->login(this->userName, this->password) != 0) {
        return Connection::CONN_AUTH_ERROR;
    }

    emit connect_state_changed(tr("Login OK. Retrive initial path ..."));
    if (this->ftp->pwd(this->homePath) != 0) {
        q_debug()<<"ftp home path error:";
        this->homePath = QString("/"); // set default homePath of ftp
    }
    emit connect_state_changed(tr("Connect done."));
    
    return 0;
}
int FTPConnection::disconnect()
{
    assert(this->ftp != NULL);
    int iret = this->ftp->logout();

    return iret;
}
int FTPConnection::reconnect()
{
    this->disconnect();
    delete this->ftp;
    this->ftp = NULL;
    return this->connect();
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
    int iret = this->ftp->noop();
    return iret;
}

