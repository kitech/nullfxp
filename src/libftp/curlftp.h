// curlftp.h --- 
// 
// Author: liuguangzhao
// Copyright (C) 2007-2010 liuguangzhao@users.sf.net
// URL: 
// Created: 2010-07-18 16:25:05 +0800
// Version: $Id$
// 


#ifndef _CURLFTP_H_
#define _CURLFTP_H_

#include <QtCore>
#include <QtNetwork>

#include "curl/curl.h"

class CurlFtp : public QObject
{
    Q_OBJECT;
public:
    explicit CurlFtp(QObject *parent = 0);
    virtual ~CurlFtp();

    enum {TYPE_MIN = 0, TYPE_ASCII, TYPE_EBCID, TYPE_BIN,
          TYPE_IMAGE, TYPE_LOCAL_BYTE,
          TYPE_MAX};
    int connect(const QString host, unsigned short port = 21);
    int login(const QString &user = QString(), const QString &password = QString());

private:
    CURL *curl;
};

#endif
