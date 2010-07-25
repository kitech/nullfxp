// curlftp_callback.cpp --- 
// 
// Author: liuguangzhao
// Copyright (C) 2007-2010 liuguangzhao@users.sf.net
// URL: 
// Created: 2010-07-19 20:55:34 +0800
// Version: $Id$
// 

#include <QtCore>

#include "curl/curl.h"

#include "curlftp.h"
#include "curlftp_callback.h"

// 给多少数据就要读取多少，读取不完不行。
size_t callback_read_dir(void *ptr, size_t size, size_t nmemb, void *userp)
{
    qDebug()<<__FILE__<<__LINE__<<__FUNCTION__<<size<<nmemb<<userp;
    
    size_t tlen = size * nmemb, rlen = 0;
    QBuffer strbuf;
    QByteArray line;
    int n = 0;

    strbuf.setData((const char*)ptr, tlen);
    strbuf.open(QBuffer::ReadOnly);
    Q_ASSERT(strbuf.canReadLine()); //  ???
    rlen = 0;
    while (!strbuf.atEnd()) {
        line = strbuf.readLine();
        rlen += line.length();
        qDebug()<<"Line: "<<n++<<line;
        // break;
    }
    strbuf.close();

    return rlen;
}

size_t callback_read_file(void *ptr, size_t size, size_t nmemb, void *userp)
{
    // qDebug()<<__FILE__<<__LINE__<<__FUNCTION__<<size<<nmemb<<userp;

    size_t tlen = size * nmemb, rlen = 0;
    QBuffer strbuf;
    QByteArray line;
    int n = 0, wlen = 0;

    CurlFtp *ftp = static_cast<CurlFtp*>(userp);
    QLocalSocket *router = ftp->getDataSock2();

    strbuf.setData((const char*)ptr, tlen);
    strbuf.open(QBuffer::ReadOnly);
    // Q_ASSERT(strbuf.canReadLine()); //  ???
    rlen = 0;
    while (!strbuf.atEnd()) {
        if (strbuf.canReadLine()) {
            line = strbuf.readLine();
        } else {
            line = strbuf.readAll();
        }
        rlen += line.length();
        wlen = router->write(line);
        // qDebug()<<"Line: "<<n++<<line.length()<<wlen;
        // fprintf(stdout, "%s", ".");
        // fflush(stdout);
        Q_ASSERT(line.length() == wlen);
        // break;
    }
    strbuf.close();
    router->flush();

    // qDebug()<<"can rw:"<<router->isReadable()<<router->isWritable()<<router->isOpen();
    fprintf(stdout, "route read file:. %p %d %s", router, router->bytesAvailable(), "\n");
    fflush(stdout);

    return rlen;
    return 0;
}

int gn = 0;
size_t callback_write_file(void *ptr, size_t size, size_t nmemb, void *userp)
{
    qDebug()<<__FILE__<<__LINE__<<__FUNCTION__<<size<<nmemb<<userp;

    int tlen = size * nmemb;
    char *s = (char*)ptr;

    CurlFtp *ftp = static_cast<CurlFtp*>(userp);
    QLocalSocket *router = ftp->getDataSock2();
    Q_ASSERT(router != NULL);

    if (gn == 0) {
        gn ++;
    } else {
        return 0;
    }
    for (int i = 0 ; i < tlen ; i ++) {
        s[i] = 'v';
    }

    return tlen;
}

int callback_debug(CURL *curl, curl_infotype it, char *text, size_t size, void *userp)
{
    char *str = (char*)calloc(1, size + 1);
    strncpy(str, text, size);
    str[size] = '\0';
    qDebug()<<"spy debug:"<<it<<str;

    if (it == CURLINFO_HEADER_IN) {
        CurlFtp *ftp = static_cast<CurlFtp*>(userp);
        ftp->rawRespBuff.write(str, size);
    }

    free(str);
    return 0;
}
