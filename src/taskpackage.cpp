// taskpackage.cpp --- 
// 
// Author: liuguangzhao
// Copyright (C) 2007-2010 liuguangzhao@users.sf.net
// URL: http://www.qtchina.net http://nullget.sourceforge.net
// Created: 2009-05-07 23:15:22 +0800
// Version: $Id$
// 

#include <assert.h>

#include "taskpackage.h"

TaskPackage::TaskPackage()
{
}

TaskPackage::TaskPackage(int scheme)
    : scheme(scheme)
{
}

TaskPackage::~TaskPackage()
{

}
int TaskPackage::setFile(QString file)
{
    int current_file_count = this->files.count();
    this->files.clear();
    this->files<<file;

    return current_file_count;
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

#ifndef QT_NO_DEBUG_STREAM
QDebug operator<<(QDebug dbg, const TaskPackage &pkg)
{
    dbg.nospace() <<"(TaskPackage: "
                  <<"scheme: "<<TaskPackage::getProtocolNameById(pkg.scheme)
                  <<"host info: "<<pkg.username<<":"<<pkg.password<<"@"
                  <<pkg.host<<": "<<pkg.port
                  <<"pubkey: "<<pkg.pubkey
                  <<")";
   
    return dbg.space();
}
#endif

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
    return name.toUpper(); // TODO if no problem, change above to upper plain text
}
// 成转换mimedata中可用的二进制流
QByteArray TaskPackage::toRawData()
{
    QByteArray ba;

    // int scheme;
    // QStringList files;
    // QString host;
    // QString port;
    // QString username;
    // QString password;
    // QString pubkey;

    ba = "TaskPackage|";
    for (int i = 0 ; i < this->files.count(); i ++) {
        if (i > 0) {
            ba +='&';
        }
        ba += this->files.at(i).toAscii().toHex();
    }

    ba += '|';
    ba += QString("%1").arg(this->scheme).toAscii().toHex();
    ba += '|' + this->host.toAscii().toHex();
    ba += '|' + this->port.toAscii().toHex();
    ba += '|' + this->username.toAscii().toHex();
    ba += '|' + this->password.toAscii().toHex();
    ba += '|' + this->pubkey.toAscii().toHex();

    return ba;
}

TaskPackage TaskPackage::fromRawData(QByteArray ba)
{
    QList<QByteArray> elts = ba.split('|');
    if (elts.count() != 8 || elts.at(0) != "TaskPackage") {
        qDebug()<<"Invalid TaskPackage package";
        return TaskPackage(PROTO_MAX);
    }

    TaskPackage pkg(QByteArray::fromHex(elts.at(2)).toInt());
    QList<QByteArray> files = elts.at(1).split('&');
    for (int i = 0 ; i < files.count(); i ++) {
        pkg.files<<QString(QByteArray::fromHex(files.at(i)));
    }
    pkg.host = QString(QByteArray::fromHex(elts.at(3)));
    pkg.port = QString(QByteArray::fromHex(elts.at(4)));
    pkg.username = QString(QByteArray::fromHex(elts.at(5)));
    pkg.password = QString(QByteArray::fromHex(elts.at(6)));
    pkg.pubkey = QString(QByteArray::fromHex(elts.at(7)));

    return pkg;
}

QMap<QString, QString> TaskPackage::hostInfo()
{
    QMap<QString, QString> host;

    // host["show_name"] = this->show_name;
    if (this->scheme == PROTO_FILE) {
        host["protocol"] = "FILE";
    } else if (this->scheme == PROTO_FTP) {
        host["protocol"] = "FTP";
    } else if (this->scheme == PROTO_SFTP) {
        host["protocol"] = "SFTP";
    } else {
        assert(0);
    }
    host["host_name"] = this->host;
    host["user_name"] = this->username;
    host["password"] = this->password;
    host["password"] = QUrl::toPercentEncoding(host["password"]);
    host["port"] = this->port;
    if (this->pubkey != QString::null && this->pubkey.length() > 0) {
        host["pubkey"] = this->pubkey;
    }

    return host;
}

