// log.cpp --- 
// 
// Author: liuguangzhao
// Copyright (C) 2007-2010 liuguangzhao@users.sf.net
// URL: 
// Created: 2011-05-01 16:02:41 +0800
// Version: $Id$
// 

#include <stdio.h>

#include "simplelog.h"

#ifdef WIN32
#define STDIN_FILENO 0
#define STDOUT_FILENO 1
#define STDERR_FILENO  2
#endif

boost::shared_ptr<FileLog> FileLog::mInst = boost::shared_ptr<FileLog>();
FileLog::FileLog()
    :QObject()
{
    this->mStream = new QFile(0);

    int log_to_file_mode = 1;
    log_to_file_mode = (QSettings(qApp->applicationDirPath()+"/kitphone.ini", QSettings::IniFormat)
                        .value("logtofile").toString() == "true") ? 1 : 0;
    if (log_to_file_mode) {
        if (!QDir().exists(qApp->applicationDirPath() + "/dlog")) {
            QDir().mkdir(qApp->applicationDirPath() + "/dlog");
        }
        QString log_file_name = QString("%1/dlog/kitdebug-%2.log")
            .arg(qApp->applicationDirPath())
            .arg(QDateTime::currentDateTime().toString("yyyyMMdd"));
        this->mStream->setFileName(log_file_name);
        if (!this->mStream->open(QIODevice::Append)) {
            Q_ASSERT(1==2);
        }
    } else {
        // qDebug()<<"opening stream.......";
        if (!this->mStream->open(STDERR_FILENO, QIODevice::WriteOnly)) {
            Q_ASSERT(1==2);
            qDebug()<<this->mStream->errorString();
            assert(1==2);
        }
    }
}

FileLog::~FileLog()
{
    // qDebug()<<__FILE__<<__LINE__<<__FUNCTION__;
    this->mStream->close();
    delete this->mStream;
}

QFile *FileLog::stream()
{
    return this->mStream;
}

boost::shared_ptr<FileLog> FileLog::instance()
{
    if (FileLog::mInst == boost::shared_ptr<FileLog>()) {
        boost::shared_ptr<FileLog> hlog = boost::shared_ptr<FileLog>(new FileLog());
        FileLog::mInst = hlog;
    }

    return FileLog::mInst;
}
