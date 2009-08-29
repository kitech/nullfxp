// basestorage.cpp --- 
// 
// Author: liuguangzhao
// Copyright (C) 2007-2010 liuguangzhao@users.sf.net
// URL: http://www.qtchina.net http://nullget.sourceforge.net
// Created: 2008-04-04 14:39:04 +0000
// Version: $Id$
// 

#include "basestorage.h"

BaseStorage * BaseStorage::mInstance = NULL;
BaseStorage * BaseStorage::instance()
{
    if(BaseStorage::mInstance == NULL) {
        BaseStorage::mInstance = new BaseStorage();
    }
    return BaseStorage::mInstance;
}

BaseStorage::BaseStorage()
{
    this->init();
}

BaseStorage::~BaseStorage()
{
}

bool BaseStorage::init()
{
#ifdef Q_OS_WIN
    this->storagePath = QCoreApplication::applicationDirPath() + QString("/.nullfxp/sessions");
#elif Q_OS_MAC
    this->storagePath = QDir::homePath()+QString("/.nullfxp/sessions");
#else
    this->storagePath = QDir::homePath()+QString("/.nullfxp/sessions");
#endif

    if (!QDir().exists(this->storagePath)) {
        if (!QDir().mkpath(this->storagePath)) {
            return false;
        }
    }    
    return true;
}

QString BaseStorage::getSessionPath()
{
    return this->storagePath;
}
QString BaseStorage::getConfigPath()
{
    return QDir(this->storagePath+"/..").absolutePath();
}

bool BaseStorage::addHost(QMap<QString, QString> host, QString catPath)
{
    if (catPath != QString::null && !QDir().mkpath(catPath)) {
        return false;
    }
    QString sessName = host["show_name"];
    QString sessFile = (catPath == QString::null ? this->storagePath : catPath) + "/" + sessName;
    QSettings settings(sessFile, QSettings::IniFormat);

    QMap<QString, QString>::iterator it;
    for (it = host.begin(); it != host.end(); it++) {
        settings.setValue(it.key(), it.value());
    }
    return true;
}

bool BaseStorage::removeHost(QString show_name, QString catPath)
{

    QString sessFile = (catPath == QString::null ? this->storagePath : catPath) + "/" + show_name;
    if (QFile(sessFile).exists()) {
        return QFile(sessFile).remove();
    } 
    return false;
}

bool BaseStorage::updateHost(QMap<QString,QString> host, QString newName, QString catPath)
{

    QString sessName = host["show_name"];
    if (newName == QString::null) {
        newName = sessName;
    }
    QString sessFile = (catPath == QString::null ? this->storagePath : catPath) + "/" + sessName;
    QString newSessFile = (catPath == QString::null ? this->storagePath : catPath) + "/" + newName;

    if (sessName != newName) {
        if (!QFile(sessFile).rename(newSessFile)) {
            return false;
        }
    }
    QSettings settings(newSessFile, QSettings::IniFormat);
    QMap<QString, QString>::iterator it;
    for (it = host.begin(); it != host.end(); it++) {
        settings.setValue(it.key(), it.value());
    }
    settings.setValue("show_name", newName);

    return true;
}

bool BaseStorage::containsHost(QString show_name, QString catPath)
{
    QString sessFile = (catPath == QString::null ? this->storagePath : catPath) + "/" + show_name;

    if (QFile(sessFile).exists()) {
        return true;
    }
    return false;
}

QMap<QString, QMap<QString,QString> > BaseStorage::getAllHost(QString catPath)
{
    QMap<QString, QMap<QString,QString> > hosts;

    return hosts;
}
QStringList BaseStorage::getNameList(QString catPath)
{
    QStringList nlist;

    // nlist = this->hosts.keys();

    return nlist;
}

QMap<QString,QString> BaseStorage::getHost(QString show_name, QString catPath)
{
    QMap<QString,QString> host;

    if (this->containsHost(show_name, catPath)) {
        QString sessFile = (catPath == QString::null ? this->storagePath : catPath) + "/" + show_name;
        QSettings settings(sessFile, QSettings::IniFormat);
        QStringList keys = settings.allKeys();
        for (int i = 0; i < keys.count(); i ++) {
            host[keys.at(i)] = settings.value(keys.at(i)).toString();
        }
        return host;
    }
    else {
        return host;
    }
}

int BaseStorage::hostCount(QString catPath)
{
    QString sessPath = (catPath == QString::null ? this->storagePath : catPath);
    
    return QDir(sessPath).entryList().count() - 2;
}

bool BaseStorage::clearHost(QString catPath)
{
    return true;
}

// 
// basestorage.cpp ends here

