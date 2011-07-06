// basestorage.cpp --- 
// 
// Author: liuguangzhao
// Copyright (C) 2007-2010 liuguangzhao@users.sf.net
// URL: http://www.qtchina.net http://nullget.sourceforge.net
// Created: 2008-04-04 14:39:04 +0000
// Version: $Id$
// 

#include "utils.h"

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
#ifdef Q_OS_MAC
    this->storagePath = QDir::homePath()+QString("/.nullfxp/sessions");
#else
# ifdef Q_OS_WIN
    this->storagePath = QCoreApplication::applicationDirPath() + QString("/.nullfxp/sessions");
#else
    // qDebug()<<"else????";
    this->storagePath = QDir::homePath()+QString("/.nullfxp/sessions");
#endif
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

/*
  forward.db is ini format
  [fwd_sess_name]
  fwd_sess_name=
  ref_sess_name=
  remote_port=
  remote_host=
  dest_host=
  dest_port=
  ctime=
  mtime=
 */
QString BaseStorage::getForwardConfigFilePath()
{
    return this->getConfigPath() + "/forward.db";
}

bool BaseStorage::addHost(const QMap<QString, QString> &host, const QString &catPath)
{
    if (catPath != QString::null && !QDir().mkpath(catPath)) {
        return false;
    }
    QString sessName = host["show_name"];
    QString sessFile = (catPath == QString::null ? this->storagePath : catPath) + "/" + sessName;
    QSettings settings(sessFile, QSettings::IniFormat);

    QMap<QString, QString>::const_iterator it;
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
        if (!QFile().rename(sessFile, newSessFile)) {
            q_debug()<<"rename error";
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

QMap<QString, QMap<QString,QString> > BaseStorage::getAllHost(const QString &catPath)
{
    Q_UNUSED(catPath);
    QMap<QString, QMap<QString,QString> > hosts;

    return hosts;
}
QStringList BaseStorage::getNameList(const QString &catPath)
{
    Q_UNUSED(catPath);
    QStringList nlist;
    QStringList elist;
    
    QDir::Filters filters = QDir::Files | QDir::NoDotAndDotDot;
    elist = QDir(this->storagePath).entryList(filters);
    // qDebug()<<elist;
    // nlist = this->hosts.keys();
    nlist = elist;

    return nlist;
}

QMap<QString,QString> BaseStorage::getHost(const QString &show_name, const QString &catPath)
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
    Q_UNUSED(catPath);

    return true;
}

bool BaseStorage::addForwarder(const QMap<QString, QString> &fwd)
{
    QString forward_file_path = this->getForwardConfigFilePath();
    QSettings forward_settings(forward_file_path, QSettings::IniFormat);

    QString fwd_sess_name;
    QString ref_sess_name;
    QString remote_host;
    QString remote_port;
    QString dest_host;
    QString dest_ports;
    QString ctime;
    QString mtime;

    fwd_sess_name = fwd["fwd_sess_name"];
    ref_sess_name = fwd["ref_sess_name"];
    remote_host = fwd["remote_host"];
    remote_port = fwd["remote_port"];
    dest_host = fwd["dest_host"];
    dest_ports = fwd["dest_ports"];
    mtime = QDateTime::currentDateTime().toString();

    forward_settings.beginGroup(fwd_sess_name);

    QStringList groups = forward_settings.childGroups();
    if (groups.contains(fwd_sess_name)) {
        ctime = forward_settings.value("ctime").toString();
    } else {
        ctime = mtime;
    }

    // forward_settings.beginGroup(fwd_sess_name);
    forward_settings.setValue("fwd_sess_name", fwd_sess_name);
    forward_settings.setValue("ref_sess_name", ref_sess_name);
    forward_settings.setValue("remote_host", remote_host);
    forward_settings.setValue("remote_port", remote_port);
    forward_settings.setValue("dest_host", dest_host);
    forward_settings.setValue("dest_ports", dest_ports);
    forward_settings.setValue("ctime", ctime);
    forward_settings.setValue("mtime", mtime);

    return true;
}

const QMap<QString, QString> BaseStorage::getForwarder(const QString &name)
{
    QMap<QString, QString> fwd;

    QString forward_file_path = this->getForwardConfigFilePath();
    QSettings forward_settings(forward_file_path, QSettings::IniFormat);

    QString fwd_sess_name;
    QString ref_sess_name;
    QString remote_host;
    QString remote_port;
    QString dest_host;
    QString dest_ports;
    QString ctime;
    QString mtime;

    QStringList groups = forward_settings.childGroups();
    if (groups.contains(name)) {
    } else {
        return fwd;
    }
    
    forward_settings.beginGroup(name);
    fwd_sess_name = forward_settings.value("fwd_sess_name").toString();
    ref_sess_name = forward_settings.value("ref_sess_name").toString();
    remote_host = forward_settings.value("remote_host").toString();
    remote_port = forward_settings.value("remote_port").toString();
    dest_host = forward_settings.value("dest_host").toString();
    dest_ports = forward_settings.value("dest_ports").toString();
    ctime = forward_settings.value("ctime").toString();
    mtime = forward_settings.value("mtime").toString();

    fwd["fwd_sess_name"] = fwd_sess_name;
    fwd["ref_sess_name"] = ref_sess_name;
    fwd["remote_host"] = remote_host;
    fwd["remote_port"] = remote_port;
    fwd["dest_host"] = dest_host;
    fwd["dest_ports"] = dest_ports;
    fwd["ctime"] = ctime;
    fwd["mtime"] = mtime;
    
    return fwd;
}

const QStringList BaseStorage::getForwarderNames()
{
    QString forward_file_path = this->getForwardConfigFilePath();
    QSettings forward_settings(forward_file_path, QSettings::IniFormat);
    QStringList groups = forward_settings.childGroups();

    return groups;
}

// global options method
bool BaseStorage::saveOptions(QMap<QString, QMap<QString, QString> > options)
{
    Q_UNUSED(options);
    return true;
}

bool BaseStorage::saveOptions(QString section, QMap<QString, QString> options)
{
    Q_UNUSED(section);
    Q_UNUSED(options);

    return true;
}

// 
// basestorage.cpp ends here

