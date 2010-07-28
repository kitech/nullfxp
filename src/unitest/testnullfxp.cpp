// testnullfxp.cpp --- 
// 
// Author: liuguangzhao
// Copyright (C) 2007-2010 liuguangzhao@users.sf.net
// URL: http://www.qtchina.net http://nullget.sourceforge.net
// Created: 2008-04-04 13:05:32 +0800
// Version: $Id$
// 



#include "testnullfxp.h"

#include "basestorage.h"
#include "utils.h"
#include "libssh2.h"
#include "libssh2_sftp.h"
#include "sshfileinfo.h"
#include "libftp/curlftp.h"

void TestNullfxp::initTestCase()
{
    QVERIFY(1 == 1);
}

void TestNullfxp::myFirstTest()
{
    QVERIFY(1 == 1);
}

void TestNullfxp::mySecondTest()
{
    QVERIFY(1 == 1);
}

void TestNullfxp::storageTest()
{
    BaseStorage * storage = BaseStorage::instance();

    QMap<QString, QString> host;
    host["protocol"] = "SFTP";
    host["show_name"] = "hahaha";
    host["host_name"] = "www.qtchina.net";
    host["user_name"] = "liuguangzhao";
    if(storage->containsHost(host["show_name"]))
        QVERIFY(storage->addHost(host) == false);
    else
        QVERIFY(storage->addHost(host) == true);

    host.clear();
    host["show_name"] = "hahahatttt";
    host["host_name"] = "shell.sf.net";
    host["user_name"] = "liuguangzhao6566";

    QVERIFY(storage->updateHost(host) == true);

}

void TestNullfxp::cleanupTestCase()
{
    QVERIFY(1 == 1);
}

void TestNullfxp::testSpecialFileName()
{
    QUrl u("file:///home/gzl/nxpt//#.newsrc-dribble#");
    int rc = is_reg("/home/gzl/nxpt//#.newsrc-dribble#");
    QVERIFY(rc != 0 ) ;
}

void TestNullfxp::testSSHFileInfo()
{
    LIBSSH2_SFTP_ATTRIBUTES attr;

    memset(&attr, 0, sizeof(LIBSSH2_SFTP_ATTRIBUTES));
    attr.filesize = 123456;
    QVERIFY(SSHFileInfo(attr).size() == 123456);
    attr.filesize = 1234567;
    QVERIFY(SSHFileInfo(&attr).size() != 123456);
    QVERIFY(SSHFileInfo(&attr).size() == 1234567);

    attr.permissions = S_IFDIR;
    QVERIFY(SSHFileInfo(attr).isDir() == true);
    attr.permissions = S_IFREG;
    QVERIFY(SSHFileInfo(attr).isDir() == false);
    QVERIFY(SSHFileInfo(attr).isFile() == true);

    QVERIFY(SSHFileInfo(attr).isSymLink() == false);
    attr.permissions = S_IFLNK;
    QVERIFY(SSHFileInfo(attr).isSymLink() == true);

    QVERIFY(SSHFileInfo(0).isSymLink() == false);
    QVERIFY(SSHFileInfo(0).isDir() == false);
    QVERIFY(SSHFileInfo(0).isFile() == false);
    QVERIFY(SSHFileInfo(0).size() == 0);

}

void TestNullfxp::testCXXBaseSyntax()
{
    char *str = new char[100];
    QVERIFY(str != NULL);
    delete [] str;
    int *ia = new int[50];
    QVERIFY(ia != NULL);
    QVERIFY(ia[0] != 0);
    memset(ia, 0, sizeof(int)*50);
    QVERIFY(ia[0] == 0);
    delete [] ia;
    
    ia = (int*)calloc(50, sizeof(int));
    QVERIFY(ia[0] == 0);
    free(ia);
    ia = (int*)malloc(sizeof(int)*50);
    QVERIFY(ia[0] != 0);
    memset(ia, 0, sizeof(int)*50);
    QVERIFY(ia[0] == 0);
    free(ia);
}

#ifndef _MSC_VER
#if (__GNUC__ >= 4) && (__GNUC_MINOR__ >= 4)
#include <tr1/array>
#else
    #warning "you gcc version is to lower. need gcc >= 4.4.0 for all feature."
#endif
#endif

void TestNullfxp::testCXX0XSyntax()
{
#ifndef _MSC_VER
#if (__GNUC__ >= 4) && (__GNUC_MINOR__ >= 4)
    std::tr1::array<int, 0> tr1arr;
    QVERIFY(tr1arr.size() == 0);
    tr1arr[0] = 5;
    qDebug()<<"array size:"<<tr1arr.size();
    QVERIFY(tr1arr.size() == 0);
    tr1arr[1] = 6;
    QVERIFY(tr1arr.size() == 0);
    qDebug()<<"a[1]="<<tr1arr[1];
    //这个数组模板个数很奇怪啊。
#else
    #warning "you gcc version is to lower. need gcc >= 4.4.0 for all feature."
#endif
#endif    
}


//////////////////////

class TestThread : public QThread
{
public:
    CurlFtp *ftp;
    void uploadFile()
    {
        int ret;
        int ret2;
        this->ftp->put("/mysql-5.5.5-m3.tar.gz");
        
        QString upfile = "/home/gzleo/NGDownload/mysql-5.5.5-m3.tar.gz";

        ftp->passive();
        ftp->connectDataChannel();

        QLocalSocket *dsock = ftp->getDataSock();
        char buf[160];
        QFile upfp(upfile);
        Q_ASSERT(upfp.open(QIODevice::ReadWrite) == true);
        ret = upfp.open(QIODevice::ReadOnly);
        Q_ASSERT(ret == true);

        int totalPutSize = 0;
        upfp.seek(0);
        qDebug()<<"upfp atend???"<<upfp.atEnd();
        while (!upfp.atEnd()) {
            ret = upfp.read(buf, sizeof(upfp));
            ret2 = dsock->write(buf, ret);
            dsock->flush();
            QVERIFY(ret2 == ret);
            totalPutSize += ret2;
            // qDebug()<<"put size:"<<ret;
        }
        upfp.close();

        qDebug()<<"totalPutSize:"<<totalPutSize;
        ftp->closeDataChannel();
        ftp->closeDataChannel2();
        ftp->wait();
        qDebug()<<"FTP thread finished.";
    }

    void downloadFile()
    {
        int ret;
        int ret2;

        QString upfile = "/tmp/mysql-5.5.5-m3.tar.gz";
        QFile upfp(upfile);
        ret = upfp.open(QIODevice::ReadWrite);
        Q_ASSERT(ret == true);
        upfp.resize(0);
      
        QByteArray line;

        ftp->passive();
        ftp->connectDataChannel();

        QLocalSocket *dsock = ftp->getDataSock();
        qDebug()<<"local socet:"<<dsock<<dsock->bytesAvailable()<<dsock->isOpen();

        ftp->get("/mysql-5.5.5-m3.tar.gz");
        for (;;) {
            if (dsock->bytesAvailable() > 0) {
            } else {
                dsock->waitForReadyRead(3000);
                // qDebug()<<"wait for ready read..."<<dsock->errorString()<<dsock->isOpen()<<ftp->isFinished()<<ftp->isRunning();
                if (ftp->isFinished()) {
                    // ftp->asynRunRetrDone();
                    ftp->closeDataChannel();
                    ftp->closeDataChannel2();
                    break;
                }
            }
            while (dsock->bytesAvailable() > 0) {
                if (dsock->canReadLine()) {
                    line = dsock->readLine();
                } else {
                    line = dsock->read(123);
                }
                upfp.write(line);
                // qDebug()<<"main read file data:"<<line.length()
                //     <<dsock->bytesAvailable()<<dsock->errorString()<<line;
            }
        }
        upfp.flush();
        upfp.close();
    }
    void run()
    {
        int ret;
        int ret2;
        // test curlftp
        this->ftp = new CurlFtp();
        // ftp->connect("ftp.gnu.org", 21);
        ftp->connect("localhost", 21);
        // ftp->login("ftp", "ftp@ftp.org");
        ftp->login("kitsoft", "2113");
        // ftp->type(CurlFtp::TYPE_BIN);
        // ftp->noop();

        // ftp->list("/opera.pac.js");
        // ftp->lista("/opera.pac.js");
        // ftp->mlst("/opera.pac.js");

        // ftp->list("/firefox");
        // ftp->lista("/firefox");
        // ftp->mlst("/firefox");

        // ftp->chdir("/firefox");

        QString str;
        // ftp->pwd(str);
        // qDebug()<<"PWD:   "<<str;

        // ftp->rmdir("/hahaha");
        // ftp->mkdir("/hahaha");
        // ftp->rmdir("/hahaha");

        // ftp->remove("/aa.txt");

        // ftp->rename("/110.mp3", "/110n.mp3");
        // ftp->rename("/110n.mp3", "/110.mp3");
        // ftp->rename("/新建文件夹.tar.gz", "/_淘宝shangping_.tar.gz");
        // ftp->rename("/_淘宝shangping_.tar.gz", "/新建文件夹.tar.gz");
        // ftp->system(str);
        // qDebug()<<"system type:   "<<str;

        // ftp->stat("/110.mp3");
        ftp->stat("/firefox");

        quint64 num;
        ret = ftp->size("/110.mp3", num);
        QVERIFY(ret == 0);
        ret = ftp->size("/firefox", num);
        QVERIFY(ret == 0);

        this->uploadFile();
        this->downloadFile();

        // delete ftp;
        qDebug()<<"TestThread out";
    }
};

void TestNullfxp::testCurlFtp()
{

    ///// test curl ftp functions
    // TestThread *t = new TestThread();
    // t->start();
    
    // qDebug()<<"entering qt loop.";
    // return app.exec();

    ///// test curl ftp functions
    TestThread *t = new TestThread();
    t->start();

    t->wait(-1);

}

