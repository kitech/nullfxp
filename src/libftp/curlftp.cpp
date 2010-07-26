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
    // rc = curl_easy_setopt(this->curl, CURLOPT_FTP_FILEMETHOD, CURLFTPMETHOD_SINGLECWD);
    rc = curl_easy_setopt(this->curl, CURLOPT_FTP_FILEMETHOD, CURLFTPMETHOD_NOCWD);
    rc = curl_easy_setopt(this->curl, CURLOPT_FILETIME, 0); // if ftp put, this cmd error, the put will not exec

#ifdef CURLOPT_WILDCARDMATCH
    // <= 7.19 problem
    rc = curl_easy_setopt(this->curl, CURLOPT_WILDCARDMATCH, 0);
#endif
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

    struct curl_slist *headers = NULL;
    
    QString cmd;
    QString uri = this->baseUrl;

    cmd = QString("NOOP");
    headers = curl_slist_append(headers, cmd.toAscii().data());

    curl_easy_setopt(this->curl, CURLOPT_NOBODY, 1);
    curl_easy_setopt(this->curl, CURLOPT_POSTQUOTE, headers);
    curl_easy_setopt(this->curl, CURLOPT_URL, uri.toAscii().data());

    // res = curl_easy_setopt(this->curl, CURLOPT_URL, "ftp://localhost");
    res = curl_easy_setopt(this->curl, CURLOPT_CONNECT_ONLY, 0);

    // res = curl_easy_setopt(this->curl, CURLOPT_URL, "ftp://localhost/gnu/");
    // res = curl_easy_setopt(this->curl, CURLOPT_WRITEDATA, this);
    // res = curl_easy_setopt(this->curl, CURLOPT_WRITEFUNCTION, callback_read_dir);
    res = curl_easy_perform(this->curl);
    qDebug()<<"after call perform"<<this;
    // res = curl_easy_setopt(this->curl, CURLOPT_WRITEDATA, ::stdout);
    // res = curl_easy_setopt(this->curl, CURLOPT_WRITEFUNCTION, NULL);

    // curl_easy_setopt(this->curl, CURLOPT_DIRLISTONLY, 0);
    // curl_easy_setopt(this->curl, CURLOPT_CUSTOMREQUEST, NULL);
    curl_easy_setopt(this->curl, CURLOPT_POSTQUOTE, NULL);
    curl_easy_setopt(this->curl, CURLOPT_NOBODY, 0);

    // res = curl_easy_setopt(this->curl, CURLOPT_POSTQUOTE, headers);
    // res = curl_easy_perform(this->curl);
    // res = curl_easy_setopt(this->curl, CURLOPT_POSTQUOTE, NULL);

    curl_slist_free_all(headers);

    long rcode = 0;
    curl_easy_getinfo(this->curl, CURLINFO_RESPONSE_CODE, &rcode);
    qDebug()<<"login code:"<<rcode;
    Q_ASSERT(rcode == 200); // is noop's response code

    // infomation....

    return 0;
}

int CurlFtp::logout()
{
    return 0;
}

int CurlFtp::connectDataChannel()
{
    QDir().remove(QDir::tempPath() + "/" + this->routerName());
    this->curlWriteDataRouteServer = new QLocalServer();
    this->curlWriteDataRouteServer->listen(this->routerName());

    this->qdsock2 = new QLocalSocket();
    this->qdsock2->connectToServer(this->routerName());
    
    this->qdsock = NULL;
    if (this->curlWriteDataRouteServer->waitForNewConnection(5)) {
        this->qdsock = this->curlWriteDataRouteServer->nextPendingConnection();
    }
    this->qdsock2->waitForConnected();
    qDebug()<<"can rw:"<<this->qdsock->isReadable()<<this->qdsock->isWritable()
            <<this->qdsock<<this->qdsock2;

    return 0;
}

int CurlFtp::closeDataChannel()
{
    qDebug()<<"runn donevvvvvvvvvvvv8555555558888888 ";    
    this->qdsock->flush();
    this->qdsock->close();
    delete this->qdsock;
    this->qdsock = NULL;

    return 0;
}

int CurlFtp::closeDataChannel2()
{
    this->qdsock2->close();
    delete this->qdsock2;
    this->qdsock2 = NULL;

    this->curlWriteDataRouteServer->close();
    delete this->curlWriteDataRouteServer;
    this->curlWriteDataRouteServer = NULL;
    
    return 0;
}

QString CurlFtp::routerName()
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

    qDebug()<<"runn start vvttttttttvrunrunrurnurnrunnnnnnnn ";

    res = curl_easy_perform(this->curl);
    // emit this->runHere();
    qDebug()<<"runn done vvttttttttvrunrunrurnurnrunnnnnnnn ";
    
    this->closeDataChannel2();
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
// depcreated
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

int CurlFtp::list(QString path)
{
    CURLcode res;
    QString uri;
    QString cmd;
    
    // qDebug()<<"enter list......";
    uri = this->baseUrl + path;
    cmd = QString("LIST -a %1").arg(path);
    curl_easy_setopt(this->curl, CURLOPT_CUSTOMREQUEST, cmd.toAscii().data());
    curl_easy_setopt(this->curl, CURLOPT_DIRLISTONLY, 1);
    curl_easy_setopt(this->curl, CURLOPT_URL, uri.toAscii().data());

    res = curl_easy_perform(this->curl);

    curl_easy_setopt(this->curl, CURLOPT_DIRLISTONLY, 0);
    curl_easy_setopt(this->curl, CURLOPT_CUSTOMREQUEST, NULL);

    return 0;
}

int CurlFtp::lista(QString path)
{
    CURLcode res;
    QString uri;
    QString cmd;
    
    // qDebug()<<"enter list......";
    uri = this->baseUrl + path;
    cmd = QString("LIST -a %1").arg(path);
    curl_easy_setopt(this->curl, CURLOPT_CUSTOMREQUEST, cmd.toAscii().data());
    curl_easy_setopt(this->curl, CURLOPT_DIRLISTONLY, 1);
    curl_easy_setopt(this->curl, CURLOPT_URL, uri.toAscii().data());

    res = curl_easy_perform(this->curl);

    curl_easy_setopt(this->curl, CURLOPT_DIRLISTONLY, 0);
    curl_easy_setopt(this->curl, CURLOPT_CUSTOMREQUEST, NULL);

    return 0;
}

int CurlFtp::nlist(QString path)
{
    return 0;
}

int CurlFtp::mlst(QString path)
{
    CURLcode res;
    QString uri;
    QString cmd;
    
    // qDebug()<<"enter list......";
    uri = this->baseUrl + path;
    cmd = QString("MLST %1").arg(path);
    curl_easy_setopt(this->curl, CURLOPT_CUSTOMREQUEST, cmd.toAscii().data());
    curl_easy_setopt(this->curl, CURLOPT_DIRLISTONLY, 1);
    curl_easy_setopt(this->curl, CURLOPT_URL, uri.toAscii().data());

    res = curl_easy_perform(this->curl);

    curl_easy_setopt(this->curl, CURLOPT_DIRLISTONLY, 0);
    curl_easy_setopt(this->curl, CURLOPT_CUSTOMREQUEST, NULL);


    return 0;
}

int CurlFtp::pwd(QString &path) // returned path
{
    CURLcode res;
    QString uri;
    QString cmd;
    curl_slist *headers = NULL;

    // qDebug()<<"enter list......";
    uri = this->baseUrl;
    headers = curl_slist_append(headers, "PWD");
    cmd = QString("PWD");
    // curl_easy_setopt(this->curl, CURLOPT_CUSTOMREQUEST, cmd.toAscii().data());
    // curl_easy_setopt(this->curl, CURLOPT_DIRLISTONLY, 1);
    curl_easy_setopt(this->curl, CURLOPT_NOBODY, 1);
    curl_easy_setopt(this->curl, CURLOPT_POSTQUOTE, headers);
    curl_easy_setopt(this->curl, CURLOPT_URL, uri.toAscii().data());

    curl_easy_setopt(this->curl, CURLOPT_DEBUGDATA, this);
    curl_easy_setopt(this->curl, CURLOPT_DEBUGFUNCTION, callback_debug);
    Q_ASSERT(!this->rawRespBuff.isOpen());
    this->rawRespBuff.open(QIODevice::ReadWrite | QIODevice::Unbuffered);

    res = curl_easy_perform(this->curl);

    long rcode = 0;
    curl_easy_getinfo(this->curl, CURLINFO_RESPONSE_CODE, &rcode);
    qDebug()<<"pwd code:"<<rcode;
    Q_ASSERT(rcode == 257);

    // curl_easy_setopt(this->curl, CURLOPT_DIRLISTONLY, 0);
    // curl_easy_setopt(this->curl, CURLOPT_CUSTOMREQUEST, NULL);
    curl_easy_setopt(this->curl, CURLOPT_POSTQUOTE, NULL);
    curl_easy_setopt(this->curl, CURLOPT_NOBODY, 0);

    curl_easy_setopt(this->curl, CURLOPT_DEBUGDATA, stdout);
    curl_easy_setopt(this->curl, CURLOPT_DEBUGFUNCTION, NULL);

    curl_slist_free_all(headers);

    // 
    qDebug()<<"abtained:"<<this->rawRespBuff.data();
    this->rawRespBuff.close();

    return 0;
}

int CurlFtp::mkdir(QString path)
{
    CURLcode res;
    struct curl_slist *headers = NULL;
    
    QString cmd;
    QString uri = this->baseUrl;

    cmd = QString("MKD %1").arg(path);
    headers = curl_slist_append(headers, cmd.toAscii().data());

    curl_easy_setopt(this->curl, CURLOPT_NOBODY, 1);
    curl_easy_setopt(this->curl, CURLOPT_POSTQUOTE, headers);
    curl_easy_setopt(this->curl, CURLOPT_URL, uri.toAscii().data());

    res = curl_easy_perform(this->curl);

    // curl_easy_setopt(this->curl, CURLOPT_DIRLISTONLY, 0);
    // curl_easy_setopt(this->curl, CURLOPT_CUSTOMREQUEST, NULL);
    curl_easy_setopt(this->curl, CURLOPT_POSTQUOTE, NULL);
    curl_easy_setopt(this->curl, CURLOPT_NOBODY, 0);

    // res = curl_easy_setopt(this->curl, CURLOPT_POSTQUOTE, headers);
    // res = curl_easy_perform(this->curl);
    // res = curl_easy_setopt(this->curl, CURLOPT_POSTQUOTE, NULL);

    curl_slist_free_all(headers);

    long rcode = 0;
    curl_easy_getinfo(this->curl, CURLINFO_RESPONSE_CODE, &rcode);
    qDebug()<<cmd<<" code:"<<rcode;
    Q_ASSERT(rcode == 257);
 
    return 0;
}

int CurlFtp::rmdir(QString path)
{
    CURLcode res;
    struct curl_slist *headers = NULL;
    
    QString cmd;
    QString uri = this->baseUrl;

    cmd = QString("RMD %1").arg(path);
    headers = curl_slist_append(headers, cmd.toAscii().data());

    curl_easy_setopt(this->curl, CURLOPT_NOBODY, 1);
    curl_easy_setopt(this->curl, CURLOPT_POSTQUOTE, headers);
    curl_easy_setopt(this->curl, CURLOPT_URL, uri.toAscii().data());

    res = curl_easy_perform(this->curl);

    // curl_easy_setopt(this->curl, CURLOPT_DIRLISTONLY, 0);
    // curl_easy_setopt(this->curl, CURLOPT_CUSTOMREQUEST, NULL);
    curl_easy_setopt(this->curl, CURLOPT_POSTQUOTE, NULL);
    curl_easy_setopt(this->curl, CURLOPT_NOBODY, 0);

    // res = curl_easy_setopt(this->curl, CURLOPT_POSTQUOTE, headers);
    // res = curl_easy_perform(this->curl);
    // res = curl_easy_setopt(this->curl, CURLOPT_POSTQUOTE, NULL);

    curl_slist_free_all(headers);

    long rcode = 0;
    curl_easy_getinfo(this->curl, CURLINFO_RESPONSE_CODE, &rcode);
    qDebug()<<cmd<<" code:"<<rcode;
    Q_ASSERT(rcode == 250);
 
    return 0;
}

int CurlFtp::chdir(QString path)
{
    CURLcode res;
    QString uri;
    QString cmd;
    curl_slist *headers = NULL;

    // qDebug()<<"enter list......";
    uri = this->baseUrl;
    cmd = QString("CWD %1").arg(path);
    headers = curl_slist_append(headers, cmd.toAscii().data());
    // curl_easy_setopt(this->curl, CURLOPT_CUSTOMREQUEST, cmd.toAscii().data());
    // curl_easy_setopt(this->curl, CURLOPT_DIRLISTONLY, 1);
    curl_easy_setopt(this->curl, CURLOPT_NOBODY, 1);
    curl_easy_setopt(this->curl, CURLOPT_POSTQUOTE, headers);
    curl_easy_setopt(this->curl, CURLOPT_URL, uri.toAscii().data());

    curl_easy_setopt(this->curl, CURLOPT_DEBUGDATA, this);
    curl_easy_setopt(this->curl, CURLOPT_DEBUGFUNCTION, callback_debug);
    Q_ASSERT(!this->rawRespBuff.isOpen());
    this->rawRespBuff.open(QIODevice::ReadWrite | QIODevice::Unbuffered);

    res = curl_easy_perform(this->curl);

    long rcode = 0;
    curl_easy_getinfo(this->curl, CURLINFO_RESPONSE_CODE, &rcode);
    qDebug()<<"pwd code:"<<rcode;
    Q_ASSERT(rcode == 250);

    // curl_easy_setopt(this->curl, CURLOPT_DIRLISTONLY, 0);
    // curl_easy_setopt(this->curl, CURLOPT_CUSTOMREQUEST, NULL);
    curl_easy_setopt(this->curl, CURLOPT_POSTQUOTE, NULL);
    curl_easy_setopt(this->curl, CURLOPT_NOBODY, 0);

    curl_easy_setopt(this->curl, CURLOPT_DEBUGDATA, stdout);
    curl_easy_setopt(this->curl, CURLOPT_DEBUGFUNCTION, NULL);

    curl_slist_free_all(headers);

    // 
    qDebug()<<"abtained:"<<this->rawRespBuff.data();
    this->rawRespBuff.close();

    return 0;
}

int CurlFtp::put(const QString fileName)
{
    qDebug()<<"Enter put.....";
    CURLcode res;
    QString uri;

    // QDir().remove(QDir::tempPath() + "/" + this->routerName());
    // this->curlWriteDataRouteServer = new QLocalServer();
    // this->curlWriteDataRouteServer->listen(this->routerName());

    // this->qdsock2 = new QLocalSocket();
    // this->qdsock2->connectToServer(this->routerName());
    
    // this->qdsock = NULL;
    // if (this->curlWriteDataRouteServer->waitForNewConnection(5)) {
    //     this->qdsock = this->curlWriteDataRouteServer->nextPendingConnection();
    // }
    // this->qdsock2->waitForConnected();
    // qDebug()<<"can rw:"<<this->qdsock->isReadable()<<this->qdsock->isWritable()
    //         <<this->qdsock<<this->qdsock2;


    uri = this->baseUrl + QString("%1").arg(fileName);

    // res = curl_easy_setopt(this->curl, CURLOPT_URL, "ftp://ftp.gnu.org/gnu/webstump.README");
    // res = curl_easy_setopt(this->curl, CURLOPT_URL, "ftp://ftp.gnu.org/gnu/bash/readline-5.1.tar.gz");
    // res = curl_easy_setopt(this->curl, CURLOPT_URL, "ftp://ftp.gnu.org/gnu/bash/bash-1.14.4-1.14.5.diffs");
    // res = curl_easy_setopt(this->curl, CURLOPT_URL, "ftp://localhost/ttttttttt.txt");
    res = curl_easy_setopt(this->curl, CURLOPT_URL, uri.toAscii().data());
    res = curl_easy_setopt(this->curl, CURLOPT_UPLOAD, 1L);
    res = curl_easy_setopt(this->curl, CURLOPT_INFILESIZE_LARGE, 1236);
    res = curl_easy_setopt(this->curl, CURLOPT_READDATA, this);
    res = curl_easy_setopt(this->curl, CURLOPT_READFUNCTION, callback_write_file);
    // res = curl_easy_setopt(this->curl, CURLOPT_TIMEOUT, 5);

    qDebug()<<"normal ftp thread:"<<this->thread();
    Q_ASSERT(!this->isRunning());
    this->start();

    // res = curl_easy_perform(this->curl);

    qDebug()<<"after call perform long time"<<this;

    return 0;
}

int CurlFtp::remove(const QString path)
{
    CURLcode res;
    struct curl_slist *headers = NULL;
    
    QString cmd;
    QString uri = this->baseUrl;

    cmd = QString("DELE %1").arg(path);
    headers = curl_slist_append(headers, cmd.toAscii().data());

    curl_easy_setopt(this->curl, CURLOPT_NOBODY, 1);
    curl_easy_setopt(this->curl, CURLOPT_POSTQUOTE, headers);
    curl_easy_setopt(this->curl, CURLOPT_URL, uri.toAscii().data());

    res = curl_easy_perform(this->curl);

    // curl_easy_setopt(this->curl, CURLOPT_DIRLISTONLY, 0);
    // curl_easy_setopt(this->curl, CURLOPT_CUSTOMREQUEST, NULL);
    curl_easy_setopt(this->curl, CURLOPT_POSTQUOTE, NULL);
    curl_easy_setopt(this->curl, CURLOPT_NOBODY, 0);

    // res = curl_easy_setopt(this->curl, CURLOPT_POSTQUOTE, headers);
    // res = curl_easy_perform(this->curl);
    // res = curl_easy_setopt(this->curl, CURLOPT_POSTQUOTE, NULL);

    curl_slist_free_all(headers);

    long rcode = 0;
    curl_easy_getinfo(this->curl, CURLINFO_RESPONSE_CODE, &rcode);
    qDebug()<<cmd<<" code:"<<rcode;
    Q_ASSERT(rcode == 250);
 
    return 0;

}

int CurlFtp::rename(const QString src, const QString dest)
{
    CURLcode res;
    struct curl_slist *headers = NULL;
    long rcode = 0;
    
    QString cmd;
    QString uri = this->baseUrl;

    cmd = QString("RNFR %1").arg(src);
    headers = curl_slist_append(headers, cmd.toAscii().data());

    curl_easy_setopt(this->curl, CURLOPT_NOBODY, 1);
    curl_easy_setopt(this->curl, CURLOPT_POSTQUOTE, headers);
    curl_easy_setopt(this->curl, CURLOPT_URL, uri.toAscii().data());

    // send RNFM
    res = curl_easy_perform(this->curl);

    curl_slist_free_all(headers);
    headers = NULL;
    curl_easy_getinfo(this->curl, CURLINFO_RESPONSE_CODE, &rcode);
    qDebug()<<cmd<<" code:"<<rcode;
    Q_ASSERT(rcode == 350);

    //////////////////////////////// RNTO
    cmd = QString("RNTO %1").arg(dest);
    headers = curl_slist_append(headers, cmd.toAscii().data());
    curl_easy_setopt(this->curl, CURLOPT_POSTQUOTE, headers);

    res = curl_easy_perform(this->curl);

    // curl_easy_setopt(this->curl, CURLOPT_DIRLISTONLY, 0);
    // curl_easy_setopt(this->curl, CURLOPT_CUSTOMREQUEST, NULL);
    curl_easy_setopt(this->curl, CURLOPT_POSTQUOTE, NULL);
    curl_easy_setopt(this->curl, CURLOPT_NOBODY, 0);

    // res = curl_easy_setopt(this->curl, CURLOPT_POSTQUOTE, headers);
    // res = curl_easy_perform(this->curl);
    // res = curl_easy_setopt(this->curl, CURLOPT_POSTQUOTE, NULL);

    curl_slist_free_all(headers);

    curl_easy_getinfo(this->curl, CURLINFO_RESPONSE_CODE, &rcode);
    qDebug()<<cmd<<" code:"<<rcode;
    Q_ASSERT(rcode == 250);

    return 0;
}

int CurlFtp::passive()
{
    return 0;
}

int CurlFtp::type(int type)
{
    CURLcode res;
    struct curl_slist *headers = NULL;
    
    QString cmd;
    QString tname;
    QString uri = this->baseUrl;
    
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

    curl_easy_setopt(this->curl, CURLOPT_NOBODY, 1);
    curl_easy_setopt(this->curl, CURLOPT_POSTQUOTE, headers);
    curl_easy_setopt(this->curl, CURLOPT_URL, uri.toAscii().data());

    res = curl_easy_perform(this->curl);

    // curl_easy_setopt(this->curl, CURLOPT_DIRLISTONLY, 0);
    // curl_easy_setopt(this->curl, CURLOPT_CUSTOMREQUEST, NULL);
    curl_easy_setopt(this->curl, CURLOPT_POSTQUOTE, NULL);
    curl_easy_setopt(this->curl, CURLOPT_NOBODY, 0);

    // res = curl_easy_setopt(this->curl, CURLOPT_POSTQUOTE, headers);
    // res = curl_easy_perform(this->curl);
    // res = curl_easy_setopt(this->curl, CURLOPT_POSTQUOTE, NULL);

    curl_slist_free_all(headers);

    long rcode = 0;
    curl_easy_getinfo(this->curl, CURLINFO_RESPONSE_CODE, &rcode);
    qDebug()<<"pwd code:"<<rcode;
    Q_ASSERT(rcode == 200);
 
    return 0;
}

int CurlFtp::noop()
{
    CURLcode res;
    struct curl_slist *headers = NULL;
    
    QString cmd;
    QString uri = this->baseUrl;

    cmd = QString("NOOP");
    headers = curl_slist_append(headers, cmd.toAscii().data());

    curl_easy_setopt(this->curl, CURLOPT_NOBODY, 1);
    curl_easy_setopt(this->curl, CURLOPT_POSTQUOTE, headers);
    curl_easy_setopt(this->curl, CURLOPT_URL, uri.toAscii().data());

    res = curl_easy_perform(this->curl);

    // curl_easy_setopt(this->curl, CURLOPT_DIRLISTONLY, 0);
    // curl_easy_setopt(this->curl, CURLOPT_CUSTOMREQUEST, NULL);
    curl_easy_setopt(this->curl, CURLOPT_POSTQUOTE, NULL);
    curl_easy_setopt(this->curl, CURLOPT_NOBODY, 0);

    // res = curl_easy_setopt(this->curl, CURLOPT_POSTQUOTE, headers);
    // res = curl_easy_perform(this->curl);
    // res = curl_easy_setopt(this->curl, CURLOPT_POSTQUOTE, NULL);

    curl_slist_free_all(headers);

    long rcode = 0;
    curl_easy_getinfo(this->curl, CURLINFO_RESPONSE_CODE, &rcode);
    qDebug()<<cmd<<" code:"<<rcode;
    Q_ASSERT(rcode == 200);
 
    return 0;
}

int CurlFtp::system(QString &type)
{
    CURLcode res;
    QString uri;
    QString cmd;
    curl_slist *headers = NULL;

    // qDebug()<<"enter list......";
    uri = this->baseUrl;
    headers = curl_slist_append(headers, "SYST");
    cmd = QString("PWD");
    // curl_easy_setopt(this->curl, CURLOPT_CUSTOMREQUEST, cmd.toAscii().data());
    // curl_easy_setopt(this->curl, CURLOPT_DIRLISTONLY, 1);
    curl_easy_setopt(this->curl, CURLOPT_NOBODY, 1);
    curl_easy_setopt(this->curl, CURLOPT_POSTQUOTE, headers);
    curl_easy_setopt(this->curl, CURLOPT_URL, uri.toAscii().data());

    curl_easy_setopt(this->curl, CURLOPT_DEBUGDATA, this);
    curl_easy_setopt(this->curl, CURLOPT_DEBUGFUNCTION, callback_debug);
    Q_ASSERT(!this->rawRespBuff.isOpen());
    this->rawRespBuff.open(QIODevice::ReadWrite | QIODevice::Unbuffered);

    res = curl_easy_perform(this->curl);

    long rcode = 0;
    curl_easy_getinfo(this->curl, CURLINFO_RESPONSE_CODE, &rcode);
    qDebug()<<cmd<<" code:"<<rcode;
    Q_ASSERT(rcode == 215);

    // curl_easy_setopt(this->curl, CURLOPT_DIRLISTONLY, 0);
    // curl_easy_setopt(this->curl, CURLOPT_CUSTOMREQUEST, NULL);
    curl_easy_setopt(this->curl, CURLOPT_POSTQUOTE, NULL);
    curl_easy_setopt(this->curl, CURLOPT_NOBODY, 0);

    curl_easy_setopt(this->curl, CURLOPT_DEBUGDATA, stdout);
    curl_easy_setopt(this->curl, CURLOPT_DEBUGFUNCTION, NULL);

    curl_slist_free_all(headers);

    // 
    qDebug()<<"abtained:"<<this->rawRespBuff.data();
    this->rawRespBuff.close();

    return 0;
}

int CurlFtp::stat(QString path)
{
    CURLcode res;
    QString uri;
    QString cmd;
    curl_slist *headers = NULL;

    // qDebug()<<"enter list......";
    uri = this->baseUrl;
    cmd = QString("STAT %1").arg(path);
    headers = curl_slist_append(headers, cmd.toAscii().data());
    // cmd = QString("PWD");
    // curl_easy_setopt(this->curl, CURLOPT_CUSTOMREQUEST, cmd.toAscii().data());
    // curl_easy_setopt(this->curl, CURLOPT_DIRLISTONLY, 1);
    curl_easy_setopt(this->curl, CURLOPT_NOBODY, 1);
    curl_easy_setopt(this->curl, CURLOPT_POSTQUOTE, headers);
    curl_easy_setopt(this->curl, CURLOPT_URL, uri.toAscii().data());

    curl_easy_setopt(this->curl, CURLOPT_DEBUGDATA, this);
    curl_easy_setopt(this->curl, CURLOPT_DEBUGFUNCTION, callback_debug);
    Q_ASSERT(!this->rawRespBuff.isOpen());
    this->rawRespBuff.open(QIODevice::ReadWrite | QIODevice::Unbuffered);

    res = curl_easy_perform(this->curl);

    long rcode = 0;
    curl_easy_getinfo(this->curl, CURLINFO_RESPONSE_CODE, &rcode);
    qDebug()<<cmd<<" code:"<<rcode;
    Q_ASSERT(rcode == 213);

    // curl_easy_setopt(this->curl, CURLOPT_DIRLISTONLY, 0);
    // curl_easy_setopt(this->curl, CURLOPT_CUSTOMREQUEST, NULL);
    curl_easy_setopt(this->curl, CURLOPT_POSTQUOTE, NULL);
    curl_easy_setopt(this->curl, CURLOPT_NOBODY, 0);

    curl_easy_setopt(this->curl, CURLOPT_DEBUGDATA, stdout);
    curl_easy_setopt(this->curl, CURLOPT_DEBUGFUNCTION, NULL);

    curl_slist_free_all(headers);

    // 
    qDebug()<<"abtained:"<<this->rawRespBuff.data();
    this->rawRespBuff.close();

    return 0;
}

int CurlFtp::size(QString path, quint64 &siz)
{
    QString cmd;
    QByteArray respText;
    long respCode;

    cmd = QString("SIZE %1").arg(path);

    int rc = this->customCommandWithResponseText(cmd, respCode, respText);

    if (respCode == 213) {
        // parse respText here
    }

    return rc;

    // CURLcode res;
    // QString uri;
    // QString cmd;
    // curl_slist *headers = NULL;

    // // qDebug()<<"enter list......";
    // uri = this->baseUrl;
    // cmd = QString("SIZE %1").arg(path);
    // headers = curl_slist_append(headers, cmd.toAscii().data());
    // // cmd = QString("PWD");
    // // curl_easy_setopt(this->curl, CURLOPT_CUSTOMREQUEST, cmd.toAscii().data());
    // // curl_easy_setopt(this->curl, CURLOPT_DIRLISTONLY, 1);
    // curl_easy_setopt(this->curl, CURLOPT_NOBODY, 1);
    // curl_easy_setopt(this->curl, CURLOPT_POSTQUOTE, headers);
    // curl_easy_setopt(this->curl, CURLOPT_URL, uri.toAscii().data());

    // curl_easy_setopt(this->curl, CURLOPT_DEBUGDATA, this);
    // curl_easy_setopt(this->curl, CURLOPT_DEBUGFUNCTION, callback_debug);
    // Q_ASSERT(!this->rawRespBuff.isOpen());
    // this->rawRespBuff.open(QIODevice::ReadWrite | QIODevice::Unbuffered);

    // res = curl_easy_perform(this->curl);

    // long rcode = 0;
    // curl_easy_getinfo(this->curl, CURLINFO_RESPONSE_CODE, &rcode);
    // qDebug()<<cmd<<" code:"<<rcode;
    // Q_ASSERT(rcode == 213);

    // // curl_easy_setopt(this->curl, CURLOPT_DIRLISTONLY, 0);
    // // curl_easy_setopt(this->curl, CURLOPT_CUSTOMREQUEST, NULL);
    // curl_easy_setopt(this->curl, CURLOPT_POSTQUOTE, NULL);
    // curl_easy_setopt(this->curl, CURLOPT_NOBODY, 0);

    // curl_easy_setopt(this->curl, CURLOPT_DEBUGDATA, stdout);
    // curl_easy_setopt(this->curl, CURLOPT_DEBUGFUNCTION, NULL);

    // curl_slist_free_all(headers);

    // // 
    // qDebug()<<"abtained:"<<this->rawRespBuff.data();
    // this->rawRespBuff.close();

    return 0;
}

int CurlFtp::customCommand(QString cmdLine, long &respCode)
{
    CURLcode res;
    QString uri;
    QString cmd;
    curl_slist *headers = NULL;

    // qDebug()<<"enter list......";
    uri = this->baseUrl;
    cmd = cmdLine; // QString("SIZE %1").arg(path);
    headers = curl_slist_append(headers, cmd.toAscii().data());
    // cmd = QString("PWD");
    // curl_easy_setopt(this->curl, CURLOPT_CUSTOMREQUEST, cmd.toAscii().data());
    // curl_easy_setopt(this->curl, CURLOPT_DIRLISTONLY, 1);
    curl_easy_setopt(this->curl, CURLOPT_NOBODY, 1);
    curl_easy_setopt(this->curl, CURLOPT_POSTQUOTE, headers);
    curl_easy_setopt(this->curl, CURLOPT_URL, uri.toAscii().data());

    curl_easy_setopt(this->curl, CURLOPT_DEBUGDATA, this);
    curl_easy_setopt(this->curl, CURLOPT_DEBUGFUNCTION, callback_debug);
    Q_ASSERT(!this->rawRespBuff.isOpen());
    this->rawRespBuff.open(QIODevice::ReadWrite | QIODevice::Unbuffered);

    res = curl_easy_perform(this->curl);

    long rcode = 0;
    curl_easy_getinfo(this->curl, CURLINFO_RESPONSE_CODE, &rcode);
    qDebug()<<cmd<<" code:"<<rcode;
    respCode = rcode;
    // Q_ASSERT(rcode == 213);

    // curl_easy_setopt(this->curl, CURLOPT_DIRLISTONLY, 0);
    // curl_easy_setopt(this->curl, CURLOPT_CUSTOMREQUEST, NULL);
    curl_easy_setopt(this->curl, CURLOPT_POSTQUOTE, NULL);
    curl_easy_setopt(this->curl, CURLOPT_NOBODY, 0);

    curl_easy_setopt(this->curl, CURLOPT_DEBUGDATA, stdout);
    curl_easy_setopt(this->curl, CURLOPT_DEBUGFUNCTION, NULL);

    curl_slist_free_all(headers);

    // 
    qDebug()<<"abtained:"<<this->rawRespBuff.data();
    this->rawRespBuff.close();

    return 0;
}
int CurlFtp::customCommandWithResponseText(QString cmdLine, long &respCode, QByteArray &respText)
{
    int rc = customCommand(cmdLine, respCode);
    
    respText = this->rawRespBuff.data();
    this->rawRespBuff.setData(NULL, 0);
    qDebug()<<"abtained22222:"<<this->rawRespBuff.data()<<respText;
    return rc;
}
int CurlFtp::customCommandWithoutResponseText(QString cmdLine, long &respCode)
{
    int rc = customCommand(cmdLine, respCode);
    Q_ASSERT(!this->rawRespBuff.isOpen());    
    this->rawRespBuff.setData(NULL, 0);
    return rc;
}


// 为什么信号都不好使了？？？
int CurlFtp::get()
{
    qDebug()<<"Enter get.....";
    CURLcode res;

    QDir().remove(QDir::tempPath() + "/" + this->routerName());
    this->curlWriteDataRouteServer = new QLocalServer();
    this->curlWriteDataRouteServer->listen(this->routerName());

    this->qdsock2 = new QLocalSocket();
    this->qdsock2->connectToServer(this->routerName());
    
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

    QDir().remove(QDir::tempPath() + "/" + this->routerName());
    this->curlWriteDataRouteServer = new QLocalServer();
    this->curlWriteDataRouteServer->listen(this->routerName());

    this->qdsock2 = new QLocalSocket();
    this->qdsock2->connectToServer(this->routerName());
    
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
