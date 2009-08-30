// main.cpp --- 
// 
// Author: liuguangzhao
// Copyright (C) 2007-2010 liuguangzhao@users.sf.net
// URL: http://www.qtchina.net http://nullget.sourceforge.net
// Created: 2008-07-17 21:51:23 +0000
// Version: $Id$
// 

#include <QtCore>
#include <QCoreApplication>

#include "nullfxp.h"
#include "nullfxp-version.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

#if QT_VERSION >= 0x040400
    app.setApplicationVersion(NULLFXP_RELEASE);
    app.setOrganizationDomain(NULLFXP_HOMEPAGE);
    app.setOrganizationName("kitsoft");
#endif

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

