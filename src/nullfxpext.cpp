// nullfxpext.cpp --- 
// 
// Filename: nullfxpext.cpp
// Description: 
// Author: liuguangzhao
// Maintainer: 
// Copyright (C) 2007-2008 liuguangzhao <liuguangzhao@users.sf.net>
// http://www.qtchina.net
// http://nullget.sourceforge.net
// Created: 二  5月  6 22:01:49 2008 (CST)
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


#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>

#include <sys/types.h>
#include <errno.h>
#include <signal.h>

#ifdef WIN32
#include <winsock2.h>
#else
#include <sys/socket.h>
//#include <wait.h>
#endif

#include <QtNetwork>

#include <QCoreApplication>

#include "nullfxp.h"


#include "localview.h"
#include "remoteview.h"
#include "progressdialog.h"
#include "remotehostconnectingstatusdialog.h"
#include "remotehostquickconnectinfodialog.h"
#include "remotehostconnectthread.h"
#include "forwardconnectdaemon.h"

#include "synchronizeoptiondialog.h"
#include "synchronizewindow.h"

void NullFXP::slot_forward_connect(bool show)
{
    //qDebug() <<__FUNCTION__<<": "<<__LINE__<<":"<< __FILE__;
    if(fcd == 0)
    {
        fcd = new ForwardConnectDaemon(this);
        fcd->setObjectName("out");
    }
    //fcd->show();
    if(fcd->objectName() == "out" && show )
    {
        this->statusBar()->addPermanentWidget(fcd);
        fcd->setObjectName("in");
        if(!fcd->isVisible()) fcd->show();
    }
    if(fcd->objectName() == "in" && !show)
    {
        this->statusBar()->removeWidget(fcd);
        fcd->setObjectName("out");
    }
}

void NullFXP::slot_synchronize_file()
{
    //qDebug() <<__FUNCTION__<<": "<<__LINE__<<":"<< __FILE__;
    //QMessageBox::warning(this,tr("Infomation:"),tr("This feather will coming soon.") );

    SynchronizeOptionDialog * sync_dlg = new SynchronizeOptionDialog(this);
    sync_dlg->show();    
    
}

