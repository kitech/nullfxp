// basestorage.cpp --- 
// 
// Filename: basestorage.cpp
// Description: 
// Author: liuguangzhao
// Maintainer: 
// Created: 五  4月  4 14:39:04 2008 (CST)
// Version: 
// Last-Updated: 五  4月  4 16:28:26 2008 (CST)
//           By: liuguangzhao
//     Update #: 1
// URL: 
// Keywords: 
// Compatibility: 
// 
// 

// Commentary: 
// 
// 
// 
// 

// Change log:
// 
// 
// 

// Code:

#include "basestorage.h"
BaseStorage::BaseStorage()
{
}

BaseStorage::~BaseStorage()
{
    if(this->opened)
        this->close();
}

bool BaseStorage::open()
{
#ifdef Q_OS_WIN
    QString config_path = QCoreApplication::applicationDirPath() + QString("/.nullfxp");
#elif Q_OS_MAC
    //TODO should where?
    QString config_path = QDir::homePath()+QString("/.nullfxp");
#else
    QString config_path = QDir::homePath()+QString("/.nullfxp");
#endif

    if(!QDir().exists(config_path))
        if(!QDir().mkdir(config_path)) return false;

    QFile *fp = new QFile(config_path + QString("/hosts.db"));
    fp->open(QIODevice::ReadWrite|QIODevice::Unbuffered);
    this->opened = true;
    this->changed = false;
    this->ioStream.setDevice(fp);
    this->ioStream>>this->hosts;
    return true;
}

bool BaseStorage::close()
{
    QFile * fp = (QFile*)this->ioStream.device();
    if(this->changed)
        this->save();
    if(!fp->isOpen())
        fp->close();
    this->opened = false;
    return true;
    delete fp;
}

bool BaseStorage::save()
{
    if(!this->changed) return true;
    QFile * fp = (QFile*)this->ioStream.device();
    fp->resize(0);
    this->ioStream<<this->hosts;

    return true;
}
bool BaseStorage::addHost(QMap<QString,QString> host)
{
    QMap<QString, QMap<QString,QString> >::iterator it;

    for(it = this->hosts.begin(); it != this->hosts.end(); it ++)
    {
        if((*it)["show_name"] == host["show_name"]) return false;
    }
    this->hosts[host["show_name"]] = host;
    this->changed = true;
    return true;
}
bool BaseStorage::removeHost(QString show_name)
{
    if(this->containsHost(show_name))
    {
        this->hosts.remove(show_name);
        this->changed = true;
        return true;
    }
    return false;
}

bool BaseStorage::updateHost(QMap<QString,QString> host)
{
    if(this->containsHost(host["show_name"]))
        this->hosts[host["show_name"]] = host;
    else
        this->hosts[host["show_name"]] = host;
    this->changed = true;
    return true;
}
bool BaseStorage::clearHost()
{
    this->hosts.clear();
    this->save();
    return true;
}

bool BaseStorage::containsHost(QString show_name)
{
    QMap<QString, QMap<QString,QString> >::iterator it;

    for(it = this->hosts.begin(); it != this->hosts.end(); it ++)
    {
        if((*it)["show_name"] == show_name) return true;
    }
    return false;
}

QMap<QString, QMap<QString,QString> > & BaseStorage::getAllHost()
{
    if(!this->opened) 
        this->open();
    return this->hosts;
}

QMap<QString,QString> BaseStorage::getHost(QString show_name)
{
    QMap<QString,QString> host;

    if(this->containsHost(show_name))
        return this->hosts[show_name];
    else
        return host;
}

int BaseStorage::hostCount()
{
    return this->hosts.count();
}

// 
// basestorage.cpp ends here

