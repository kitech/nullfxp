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

