// connector.cpp --- 
// 
// Author: liuguangzhao
// Copyright (C) 2007-2010 liuguangzhao@users.sf.net
// URL: http://www.qtchina.net http://nullget.sourceforge.net
// Created: 2009-09-06 12:53:42 +0800
// Version: $Id$
// 

#include <assert.h>

#include "connector.h"

#include "sshconnection.h"
#include "ftpconnection.h"

Connector::Connector(QObject *parent)
    : QThread(parent)
{
    this->conn = NULL;

    this->connect_status = Connection::CONN_OK;
    this->user_canceled = false;
    ////////////////
    QObject::connect(this, SIGNAL(finished()), this, SLOT(slot_finished()));
}

Connector::~Connector()
{
}
void Connector::setHostInfo(QMap<QString, QString> host)
{
    QString protocol = host["protocol"];
    if (this->conn == NULL) {
        if (protocol == "FTPS") {
            // not impled yet
            this->conn = new FTPConnection();
        } else if (protocol == "FTPES") {
            // not impled yet
            this->conn = new FTPConnection();
        } else if (protocol == "FTP") {
            this->conn = new FTPConnection();
        } else if (protocol == "SFTP") {
            this->conn = new SSHConnection();
        } else {
            qDebug()<<"Unsupported protocol:"<<protocol;
        }
        if (this->conn != NULL) {
            QObject::connect(this->conn, SIGNAL(connect_state_changed(QString)),
                             this, SIGNAL(connect_state_changed(QString)));
        }
    }
    assert(this->conn != NULL);
    this->conn->setHostInfo(host);
}
void Connector::setUserCanceled()
{
    assert(this->conn != NULL);
    this->conn->setUserCanceled();
}

QString Connector::errorString()
{
    if (this->conn == NULL) {
        return QString();
    }
    return this->conn->errorString();
}

void Connector::run()
{
    assert(this->conn != NULL);

    int iret = this->conn->connect();
    if (iret == 0) {
    } else {
         this->connect_status = iret;
         // this->connect_status = Connection::CONN_OTHER;
    }
}

void Connector::slot_finished()
{
    emit this->connect_finished(this->connect_status, this->conn);
}

QString Connector::get_status_desc(int status)
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

