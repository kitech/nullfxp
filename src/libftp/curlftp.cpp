// curlftp.cpp --- 
// 
// Author: liuguangzhao
// Copyright (C) 2007-2010 liuguangzhao@users.sf.net
// URL: 
// Created: 2010-07-18 16:26:53 +0800
// Version: $Id$
// 

#include "curlftp.h"

static QMutex curl_global_init_mutext;
static bool curl_global_inited = false;

CurlFtp::CurlFtp(QObject *parent)
    : QObject(parent)
{
    CURLcode rc;
    if (::curl_global_inited == false) {
        curl_global_init_mutext.lock();
#if defined(Q_OS_WIN32)
        rc = curl_global_init(CURL_GLOBAL_WIN32 | CURL_GLOBAL_SSL);
#endif
        // rc = curl_global_init(CURL_GLOBAL_ALL);
        rc = curl_global_init(CURL_GLOBAL_DEFAULT);

        Q_ASSERT(rc == 0);
        curl_global_init_mutext.unlock();
    }
    this->curl = curl_easy_init();
    Q_ASSERT(this->curl != NULL);

}

CurlFtp::~CurlFtp()
{
    if (this->curl != NULL) {
        curl_easy_cleanup(this->curl);
    }
}

int CurlFtp::connect(const QString host, unsigned short port)
{
    CURLcode res;

    res = curl_easy_setopt(this->curl, CURLOPT_PORT, port);
    // res = curl_easy_setopt(this->curl, CURLOPT_FTPPORT, port);


    return 0;
}

int CurlFtp::login(const QString &user, const QString &password)
{
    CURLcode res;
    res = curl_easy_setopt(this->curl, CURLOPT_USERNAME, user.toAscii().data());

    res = curl_easy_setopt(this->curl, CURLOPT_PASSWORD, password.toAscii().data());


    return 0;
}
