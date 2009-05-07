// taskpackage.cpp --- 
// 
// Author: liuguangzhao
// Copyright (C) 2007-2010 liuguangzhao@users.sf.net
// URL: http://www.qtchina.net http://nullget.sourceforge.net
// Created: 2009-05-07 23:15:22 +0800
// Last-Updated: 
// Version: $Id$
// 

#include "taskpackage.h"

TaskPackage::TaskPackage(int scheme)
    : scheme(scheme)
{

}

TaskPackage::~TaskPackage()
{

}

void TaskPackage::dump(TaskPackage &pkg)
{
    qDebug()<<"==== task package >>>";
    qDebug()<<"scheme: "<<TaskPackage::getProtocolNameById(pkg.scheme);
    qDebug()<<"host info: "<<pkg.username<<":"<<pkg.password<<"@"
            <<pkg.host<<":"<<pkg.port;
    qDebug()<<"pubkey:"<<pkg.pubkey;
    qDebug()<<"<<<< task package ===";
}
bool TaskPackage::isValid(TaskPackage &pkg)
{
    if (pkg.scheme <= PROTO_MIN || pkg.scheme >= PROTO_MAX) {
        return false;
    }
    if (pkg.files.count() == 0) {
        return false;
    }
    return true;
}

QString TaskPackage::getProtocolNameById(int protocol_id)
{
    Q_ASSERT(protocol_id > PROTO_MIN && protocol_id < PROTO_MAX);

    QString name = "uknown";
    switch (protocol_id) {
    case PROTO_FILE:
        name = "file";
        break;
    case PROTO_SFTP:
        name = "sftp";
        break;
    case PROTO_FTP:
        name = "ftp";
        break;
    case PROTO_FTPS:
        name = "ftps";
        break;        
    case PROTO_HTTP:
        name = "http";
        break;
    case PROTO_HTTPS:
        name = "https";
        break;
    case PROTO_RSTP:
        name = "rstp";
        break;
    case PROTO_MMS:
        name = "mms";
        break;
    default:
        Q_ASSERT(1 == 2);
    };
    return name;
}
