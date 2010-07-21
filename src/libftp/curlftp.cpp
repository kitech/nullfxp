// curlftp.cpp --- 
// 
// Author: liuguangzhao
// Copyright (C) 2007-2010 liuguangzhao@users.sf.net
// URL: 
// Created: 2010-07-18 16:26:53 +0800
// Version: $Id$
// 

#include "curlftp.h"
#include "curlftp_callback.h"


static QMutex curl_global_init_mutex;
static int curl_global_inited = 0;

int CurlFtp::seq = 0;

CurlFtp::CurlFtp(QObject *parent)
    : QThread(parent)
{
    CURLcode rc;
    CurlFtp::seq += 1;

    Q_ASSERT(::curl_global_inited >= 0);
    ::curl_global_init_mutex.lock();
    if (::curl_global_inited == 0) {
#if defined(Q_OS_WIN32)
        rc = curl_global_init(CURL_GLOBAL_WIN32 | CURL_GLOBAL_SSL);
#endif
        // rc = curl_global_init(CURL_GLOBAL_ALL);
        rc = curl_global_init(CURL_GLOBAL_DEFAULT);

        Q_ASSERT(rc == 0);
        ::curl_global_inited = 1;
    } else {
        ::curl_global_inited += 1;
    }
    ::curl_global_init_mutex.unlock();

    this->curl = curl_easy_init();
    Q_ASSERT(this->curl != NULL);

    rc = curl_easy_setopt(this->curl, CURLOPT_NOSIGNAL, 1);  // this app should used in multithread envirement
    rc = curl_easy_setopt(this->curl, CURLOPT_FTP_FILEMETHOD, CURLFTPMETHOD_SINGLECWD);
    rc = curl_easy_setopt(this->curl, CURLOPT_FILETIME, 0); // if ftp put, this cmd error, the put will not exec
    rc = curl_easy_setopt(this->curl, CURLOPT_WILDCARDMATCH, 0);
    // rc = curl_easy_setopt(this->curl, CURLOPT_DNS_USE_GLOBAL_CACHE, 0); // obslate OPT
    
    this->shareHandle = curl_share_init();
    rc = curl_easy_setopt(this->curl, CURLOPT_SHARE, this->shareHandle);

    //// test
    QObject::connect(this, SIGNAL(runHere()), this, SLOT(nowarnRunDone()));
    QObject::connect(this, SIGNAL(finished()), this, SLOT(asynRunRetrDone()));
}

CurlFtp::~CurlFtp()
{
    if (this->curl != NULL) {
        curl_easy_cleanup(this->curl);
        this->curl = NULL;
    }
    if (this->shareHandle != NULL) {
        curl_share_cleanup(this->shareHandle);
        this->shareHandle = NULL;
    }

    Q_ASSERT(::curl_global_inited >= 0);
    ::curl_global_init_mutex.lock();
    ::curl_global_inited -= 1;
    if (::curl_global_inited == 0) {
        curl_global_cleanup();
        qDebug()<<"Cleanup curl global, not use ref now.";
    }
    ::curl_global_init_mutex.unlock();
}

// if need passwd, if this function success?
int CurlFtp::connect(const QString host, unsigned short port)
{
    CURLcode res;

    Q_ASSERT(!host.isEmpty());
    Q_ASSERT(port > 0);
    this->baseUrl = QString("ftp://%1:%2").arg(host).arg(port);
    qDebug()<<this->baseUrl;

    res = curl_easy_setopt(this->curl, CURLOPT_VERBOSE, 1);
    res = curl_easy_setopt(this->curl, CURLOPT_PORT, port);
    // res = curl_easy_setopt(this->curl, CURLOPT_FTPPORT, port);
    res = curl_easy_setopt(this->curl, CURLOPT_CONNECT_ONLY, 1);
    // res = curl_easy_setopt(this->curl, CURLOPT_URL, "ftp://localhost");
    res = curl_easy_setopt(this->curl, CURLOPT_URL, this->baseUrl.toAscii().data());
    
    res = curl_easy_perform(this->curl);

    qDebug()<<res<<curl_easy_strerror(res);

    // if anonymous:ftp@example, we should close this connection
    // in login call, if set a diffrent user, it will be a new connection

    return 0;
}

// can reuse if user@host:port are the same.
int CurlFtp::login(const QString &user, const QString &password)
{
    CURLcode res;

    res = curl_easy_setopt(this->curl, CURLOPT_USERNAME, user.toAscii().data());
    
    res = curl_easy_setopt(this->curl, CURLOPT_PASSWORD, password.toAscii().data());

    // res = curl_easy_setopt(this->curl, CURLOPT_URL, "ftp://localhost");
    res = curl_easy_setopt(this->curl, CURLOPT_URL, this->baseUrl.toAscii().data());

    res = curl_easy_perform(this->curl);

    res = curl_easy_setopt(this->curl, CURLOPT_CONNECT_ONLY, 0);

    // res = curl_easy_setopt(this->curl, CURLOPT_URL, "ftp://localhost/gnu/");
    res = curl_easy_setopt(this->curl, CURLOPT_URL, this->baseUrl.toAscii().data());
    res = curl_easy_setopt(this->curl, CURLOPT_WRITEDATA, this);
    res = curl_easy_setopt(this->curl, CURLOPT_WRITEFUNCTION, callback_read_dir);
    res = curl_easy_perform(this->curl);
    qDebug()<<"after call perform"<<this;
    res = curl_easy_setopt(this->curl, CURLOPT_WRITEDATA, ::stdout);
    res = curl_easy_setopt(this->curl, CURLOPT_WRITEFUNCTION, NULL);

    // infomation....

    return 0;
}

QString CurlFtp::proxyName()
{
    QString name;
    QDateTime nowTime = QDateTime::currentDateTime();

    name = QString("nullfxp_data_router_%1.sock").arg(this->seq);

    return name;
}

QLocalSocket *CurlFtp::getDataSock()
{
    Q_ASSERT(this->qdsock != NULL);
    return this->qdsock;
}

QLocalSocket *CurlFtp::getDataSock2()
{
    Q_ASSERT(this->qdsock2 != NULL);
    return this->qdsock2;
}

void CurlFtp::run()
{
    CURLcode res;

    qDebug()<<"runn donevvttttttttvrunrunrurnurnrunnnnnnnn ";

    res = curl_easy_perform(this->curl);
    // emit this->runHere();
}

void CurlFtp::nowarnRunTask()
{
    // Q_ASSERT(1 == 2);
    qDebug()<<"runn donevvttttttttv ";    
    // this method will run in new thread, by signal/slot mechinzim
    // curl_easy_perform(this->curl);
}

void CurlFtp::nowarnRunDone()
{
    // Q_ASSERT(1 == 2);
    qDebug()<<"runn donevvvvvvvvvvvv ";    
}

// need call by caller, when caller checked that this thread task is finished
void CurlFtp::asynRunRetrDone()
{
    qDebug()<<"runn donevvvvvvvvvvvv8555555558888888 ";    
    this->qdsock->flush();
    this->qdsock->close();
    delete this->qdsock;
    this->qdsock = NULL;
    this->qdsock2->close();
    delete this->qdsock2;
    this->qdsock2 = NULL;

    this->curlWriteDataRouteServer->close();
    delete this->curlWriteDataRouteServer;
    this->curlWriteDataRouteServer = NULL;
}

int CurlFtp::type(int type)
{
    CURLcode res;
    struct curl_slist *headers = NULL;
    
    QString cmd;
    QString tname;
    
    switch (type) {
    case TYPE_ASCII:
        tname = "A";
        break;
    case TYPE_EBCID:
    case TYPE_BIN:
    case TYPE_IMAGE:
    case TYPE_LOCAL_BYTE:
    default:
        tname = "I";
        break;
    }

    cmd = QString("TYPE %1").arg(tname);
    headers = curl_slist_append(headers, cmd.toAscii().data());

    res = curl_easy_setopt(this->curl, CURLOPT_POSTQUOTE, headers);
    res = curl_easy_perform(this->curl);
    res = curl_easy_setopt(this->curl, CURLOPT_POSTQUOTE, NULL);

    curl_slist_free_all(headers);

    long respCode = 0;
    res = curl_easy_getinfo(this->curl, CURLINFO_RESPONSE_CODE, &respCode);
    qDebug()<<"type response code:"<<res;

    return 0;
}

int CurlFtp::noop()
{
    CURLcode res;
    struct curl_slist *headers = NULL;
    
    QString cmd;

    cmd = QString("NOOP");
    headers = curl_slist_append(headers, cmd.toAscii().data());

    // res = curl_easy_setopt(this->curl, CURLOPT_CUSTOMREQUEST, "NOOP");
    res = curl_easy_setopt(this->curl, CURLOPT_POSTQUOTE, headers);
    res = curl_easy_perform(this->curl);
    res = curl_easy_setopt(this->curl, CURLOPT_POSTQUOTE, NULL);
    // res = curl_easy_setopt(this->curl, CURLOPT_CUSTOMREQUEST, NULL);

    curl_slist_free_all(headers);

    long respCode = 0;
    res = curl_easy_getinfo(this->curl, CURLINFO_RESPONSE_CODE, &respCode);
    qDebug()<<"type response code:"<<res;

    return 0;
}


// 为什么信号都不好使了？？？
int CurlFtp::get()
{
    qDebug()<<"Enter get.....";
    CURLcode res;

    QDir().remove(QDir::tempPath() + "/" + this->proxyName());
    this->curlWriteDataRouteServer = new QLocalServer();
    this->curlWriteDataRouteServer->listen(this->proxyName());

    this->qdsock2 = new QLocalSocket();
    this->qdsock2->connectToServer(this->proxyName());
    
    this->qdsock = NULL;
    if (this->curlWriteDataRouteServer->waitForNewConnection(5)) {
        this->qdsock = this->curlWriteDataRouteServer->nextPendingConnection();
    }
    this->qdsock2->waitForConnected();
    qDebug()<<"can rw:"<<this->qdsock->isReadable()<<this->qdsock->isWritable()
            <<this->qdsock<<this->qdsock2;
    // res = curl_easy_setopt(this->curl, CURLOPT_URL, "ftp://ftp.gnu.org/gnu/webstump.README");
    // res = curl_easy_setopt(this->curl, CURLOPT_URL, "ftp://ftp.gnu.org/gnu/bash/readline-5.1.tar.gz");
    res = curl_easy_setopt(this->curl, CURLOPT_URL, "ftp://ftp.gnu.org/gnu/bash/bash-1.14.4-1.14.5.diffs");
    res = curl_easy_setopt(this->curl, CURLOPT_WRITEDATA, this);
    res = curl_easy_setopt(this->curl, CURLOPT_WRITEFUNCTION, callback_read_file);

    qDebug()<<"normal ftp thread:"<<this->thread();
    Q_ASSERT(!this->isRunning());
    this->start();
    // emit this->runHere();

    // res = curl_easy_perform(this->curl);

    qDebug()<<"after call perform long time"<<this;
    
    // this->getInfoDemo();

    // this->qdsock->flush();
    // this->qdsock->close();
    // delete this->qdsock;
    // this->qdsock = NULL;
    // this->curlWriteDataRouteServer->close();
    // delete this->curlWriteDataRouteServer;
    // this->curlWriteDataRouteServer = NULL;
    
    return 0;
}

int CurlFtp::put()
{
    qDebug()<<"Enter put.....";
    CURLcode res;

    QDir().remove(QDir::tempPath() + "/" + this->proxyName());
    this->curlWriteDataRouteServer = new QLocalServer();
    this->curlWriteDataRouteServer->listen(this->proxyName());

    this->qdsock2 = new QLocalSocket();
    this->qdsock2->connectToServer(this->proxyName());
    
    this->qdsock = NULL;
    if (this->curlWriteDataRouteServer->waitForNewConnection(5)) {
        this->qdsock = this->curlWriteDataRouteServer->nextPendingConnection();
    }
    this->qdsock2->waitForConnected();
    qDebug()<<"can rw:"<<this->qdsock->isReadable()<<this->qdsock->isWritable()
            <<this->qdsock<<this->qdsock2;
    // res = curl_easy_setopt(this->curl, CURLOPT_URL, "ftp://ftp.gnu.org/gnu/webstump.README");
    // res = curl_easy_setopt(this->curl, CURLOPT_URL, "ftp://ftp.gnu.org/gnu/bash/readline-5.1.tar.gz");
    // res = curl_easy_setopt(this->curl, CURLOPT_URL, "ftp://ftp.gnu.org/gnu/bash/bash-1.14.4-1.14.5.diffs");
    res = curl_easy_setopt(this->curl, CURLOPT_URL, "ftp://localhost/ttttttttt.txt");
    res = curl_easy_setopt(this->curl, CURLOPT_UPLOAD, 1L);
    res = curl_easy_setopt(this->curl, CURLOPT_INFILESIZE_LARGE, 1236);
    res = curl_easy_setopt(this->curl, CURLOPT_READDATA, this);
    res = curl_easy_setopt(this->curl, CURLOPT_READFUNCTION, callback_write_file);

    qDebug()<<"normal ftp thread:"<<this->thread();
    Q_ASSERT(!this->isRunning());
    // this->start();

    res = curl_easy_perform(this->curl);

    qDebug()<<"after call perform long time"<<this;

    return 0;
}

int CurlFtp::getInfoDemo()
{
    CURLcode res;

    long intv;
    double num;
    char str[1024] = {0};
    char *ptr = NULL;
    struct curl_slist list;

    res = curl_easy_getinfo(this->curl, CURLINFO_EFFECTIVE_URL, str);
    qDebug()<<res<<"EFFECTIVE_URL: "<<QString(str)<<strlen(str);

    res = curl_easy_getinfo(this->curl, CURLINFO_RESPONSE_CODE, &intv);
    qDebug()<<res<<"RESPONSE_CODE: "<<intv;

    res = curl_easy_getinfo(this->curl, CURLINFO_HTTP_CONNECTCODE, &intv);
    qDebug()<<res<<"HTTP_CONNECTCODE: "<<intv;

    res = curl_easy_getinfo(this->curl, CURLINFO_FILETIME, &intv);
    qDebug()<<res<<"FILETIME: "<<intv;
    if (intv == -1) {
        
    }

    res = curl_easy_getinfo(this->curl, CURLINFO_TOTAL_TIME, &num);
    qDebug()<<res<<"TOTAL_TIME: "<<num;

    res = curl_easy_getinfo(this->curl, CURLINFO_NAMELOOKUP_TIME, &num);
    qDebug()<<res<<"NAMELOOKUP_TIME: "<<num;

    res = curl_easy_getinfo(this->curl, CURLINFO_CONNECT_TIME, &num);
    qDebug()<<res<<"CONNECT_TIME: "<<num;

    res = curl_easy_getinfo(this->curl, CURLINFO_APPCONNECT_TIME, &num);
    qDebug()<<res<<"APPCONNECT_TIME: "<<num;

    res = curl_easy_getinfo(this->curl, CURLINFO_PRETRANSFER_TIME, &num);
    qDebug()<<res<<"PRETRANSFER_TIME: "<<num;

    res = curl_easy_getinfo(this->curl, CURLINFO_STARTTRANSFER_TIME, &num);
    qDebug()<<res<<"STARTTRANSFER_TIME: "<<num;

    res = curl_easy_getinfo(this->curl, CURLINFO_REDIRECT_TIME, &num);
    qDebug()<<res<<"REDIRECT_TIME: "<<num;

    res = curl_easy_getinfo(this->curl, CURLINFO_REDIRECT_COUNT, &intv);
    qDebug()<<res<<"REDIRECT_COUNT: "<<intv;
    res = curl_easy_getinfo(this->curl, CURLINFO_REDIRECT_URL, &str);
    qDebug()<<res<<"REDIRECT_URL: "<<str;



    return 0;
}
