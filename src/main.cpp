// main.cpp --- 
// 
// Author: liuguangzhao
// Copyright (C) 2007-2010 liuguangzhao@users.sf.net
// URL: http://www.qtchina.net http://nullget.sourceforge.net
// Created: 2008-07-17 21:51:23 +0000
// Version: $Id$
// 

#include <QtCore>
#include <QtNetwork>
#include <QCoreApplication>
#include <QMessageBox>

#include "nullfxp.h"
#include "nullfxp-version.h"

#include "libftp/curlftp.h"

class TestThread : public QThread
{
public:
    void run()
    {
        // test curlftp
        CurlFtp *ftp = new CurlFtp();
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
        ftp->system(str);
        qDebug()<<"system type:   "<<str;

        // ftp->put();

        // ftp->get();
        
        // QByteArray line;
        // QLocalSocket *dsock = ftp->getDataSock();
        // qDebug()<<"local socet:"<<dsock<<dsock->bytesAvailable()<<dsock->isOpen();
        // for (;;) {
        //     if (dsock->bytesAvailable() > 0) {
        //     } else {
        //         dsock->waitForReadyRead(3000);
        //         qDebug()<<"wait for ready read..."<<dsock->errorString()<<dsock->isOpen()<<ftp->isFinished()<<ftp->isRunning();
        //         if (ftp->isFinished()) {
        //             ftp->asynRunRetrDone();
        //             break;
        //         }
        //     }
        //     while (dsock->bytesAvailable() > 0) {
        //         if (dsock->canReadLine()) {
        //             line = dsock->readLine();
        //         } else {
        //             line = dsock->read(123);
        //         }
        //         qDebug()<<"main read file data:"<<line.length()
        //                 <<dsock->bytesAvailable()<<dsock->errorString()<<line;
        //     }
        // }

        // delete ftp;
        qDebug()<<"TestThread out";
    }
};

int main(int argc, char *argv[])
{
    // QT_REQUIRE_VERSION(argc, argv, "4.0.0"); // has a compile warning
    QApplication app(argc, argv);

    if (strcmp(qVersion(), "4.4.0") >= 0) {
        app.setApplicationVersion(NULLFXP_RELEASE);
        app.setOrganizationDomain(NULLFXP_HOMEPAGE);
        app.setOrganizationName("kitsoft");
    }
	app.addLibraryPath(app.applicationDirPath() + "/plugins");
    app.addLibraryPath(app.applicationDirPath() + "../lib64/plugins");
    app.addLibraryPath(app.applicationDirPath() + "../lib/plugins");

    ///// test curl ftp functions
    TestThread *t = new TestThread();
    t->start();
    
    qDebug()<<"entering qt loop.";
    return app.exec();

    NullFXP nfxp;
    nfxp.showNormal();
    
    int ret = app.exec();
    if (ret == 0) {
        qDebug()<<"Exit normally.";
    } else {
        qDebug()<<"App exit with error code:"<<ret;
    }
    return ret;
}

