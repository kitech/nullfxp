// connector.cpp --- 
// 
// Author: liuguangzhao
// Copyright (C) 2007-2010 liuguangzhao@users.sf.net
// URL: http://www.qtchina.net http://nullget.sourceforge.net
// Created: 2009-09-06 12:53:42 +0800
// Version: $Id$
// 


#include "connector.h"

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
    if (this->conn == NULL) {
        this->conn = new FTPConnection();
    }
    this->conn->setHostInfo(host);
}
void Connector::run()
{
    if (this->conn == NULL) {
        this->conn = new FTPConnection();
    }

    int iret = this->conn->connect();
    if (iret == 0) {

    } else {
        this->connect_status = Connection::CONN_OTHER;
    }
}

void Connector::slot_finished()
{
    emit this->connect_finished(this->connect_status, this->conn);
}
