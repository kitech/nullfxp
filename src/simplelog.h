// simplelog.h --- 
// 
// Author: liuguangzhao
// Copyright (C) 2007-2010 liuguangzhao@users.sf.net
// URL: 
// Created: 2011-05-01 15:45:06 +0800
// Version: $Id$
// 


#ifndef _SIMPLE_LOG_H_
#define _SIMPLE_LOG_H_

#include <QtCore>

#include "boost/smart_ptr.hpp"

class FileLog : public QObject
{
    Q_OBJECT;
public:
    virtual ~FileLog();
    static boost::shared_ptr<FileLog> instance();
    QFile *stream();

protected:
    explicit FileLog();

private:
    static boost::shared_ptr<FileLog> mInst;
    QFile* mStream;
};

class XQDebug : public QDebug
{
public:
    XQDebug(QIODevice *device) : QDebug(device) {
    }

    ~XQDebug() {
        #ifdef WIN32
        *this<<"\r\n";
        #else
        *this<<"\n";
        #endif
    }
};

// 不错
#define qLogx() XQDebug(FileLog::instance()->stream())<<"["<<QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz")<<"]"<<__FILE__<<__LINE__<<__FUNCTION__

#endif /* _LOG_H_ */








