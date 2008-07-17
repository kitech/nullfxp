// main.cpp --- 
// 
// Filename: main.cpp
// Description: 
// Author: 刘光照<liuguangzhao@users.sf.net>
// Maintainer: 
// Copyright (C) 2007-2008 liuguangzhao <liuguangzhao@users.sf.net>
// http://www.qtchina.net
// http://nullget.sourceforge.net
// Created: 四  7月 17 21:51:23 2008 (CST)
// Version: 
// Last-Updated: 
//           By: 
//     Update #: 0
// URL: 
// Keywords: 
// Compatibility: 
// 
// 

// Commentary: 
// 
// 
// 
// 

// Change log:
// 
// 
// 
#include <sys/types.h>

#ifdef WIN32
#include <winsock2.h>
#else
#include <sys/socket.h>
#include <arpa/inet.h>
#endif

#include <QtCore>
#include <QCoreApplication>
#include "nullfxp.h"
#include "nullfxp-version.h"

#include "baserfsmodel.h"

int main(int argc, char *argv[])
{
    int app_exec_ret = -1;
      QApplication app(argc, argv);
#if QT_VERSION >= 0x040400
      app.setApplicationVersion(NULLFXP_RELEASE);
      app.setOrganizationDomain(NULLFXP_HOMEPAGE);
      app.setOrganizationName("kitsoft");
#endif
      NullFXP nfxp ;
      nfxp.showNormal ();
      //BaseRFSModel *rfs_model = new BaseRFSModel();
      app_exec_ret = app.exec();
      if( app_exec_ret == 0)
          qDebug()<<"Exit normally.";
      else
          qDebug()<<"App exit with error code:"<<app_exec_ret;
      return app_exec_ret;
}

