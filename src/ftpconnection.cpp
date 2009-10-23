// ftpconnection.cpp --- 
// 
// Author: liuguangzhao
// Copyright (C) 2007-2010 liuguangzhao@users.sf.net
// URL: http://www.qtchina.net http://nullget.sourceforge.net
// Created: 2009-09-06 10:35:47 +0800
// Version: $Id$
// 

#include <assert.h>

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
        emit connect_state_changed(tr("Connect error: ") + this->ftp->errorString());
        return Connection::CONN_OTHER;
    }
    emit connect_state_changed(tr("Connect OK %1:%2. Login %3@%4 ...")
                               .arg(this->hostName).arg(this->port)
                               .arg(this->userName).arg(this->hostName));
    if (this->ftp->login(this->userName, this->password) != 0) {
        emit connect_state_changed(tr("Connect error: ") + this->ftp->errorString());
        return Connection::CONN_AUTH_ERROR;
    }

    emit connect_state_changed(tr("Login OK. Retrive initial path ..."));
    if (this->ftp->pwd(this->homePath) != 0) {
        q_debug()<<"ftp home path error:";
        this->homePath = QString("/"); // set default homePath of ftp
        emit connect_state_changed(tr("Connect error:") + this->ftp->errorString());
    }
    emit connect_state_changed(tr("Connect done."));
    
    // 自动取到编码后,把这个值传递给底层FTP库.
    this->codec = this->codecForEnv(QString());
    if (this->codec != NULL) {
        this->ftp->setEncoding(this->codec->name());
    }

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

QTextCodec *FTPConnection::codecForEnv(QString env)
{
    assert(this->ftp != NULL);
    QTextCodec *ecodec = NULL;
    QString stype;
    int iret = this->ftp->system(stype);
    if (iret == 0) {
        // Supported values are "UNIX", "VMS", "WINDOWS", "OS/2", "OS/400", "MVS".
        if (stype == "UNIX") {
            ecodec = QTextCodec::codecForName("UTF-8");
        } else if (stype == "WINDOWS") {
            ecodec = QTextCodec::codecForName("GBK");
        } else {
            qDebug()<<"Unknown systype:"<<stype;
        }
    }
    return ecodec;
}
