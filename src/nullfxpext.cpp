// nullfxpext.cpp --- 
// 
// Author: liuguangzhao
// Copyright (C) 2007-2010 liuguangzhao@users.sf.net
// URL: http://www.qtchina.net http://nullget.sourceforge.net
// Created: 2008-05-06 22:01:49 +0800
// Version: $Id$
// 

#include <cstdlib>
#include <cstdio>
#include <sys/stat.h>
#include <sys/types.h>
#include <cerrno>

#ifdef WIN32
#include <winsock2.h>
#else
#include <sys/socket.h>
//#include <wait.h>
#endif

#include <QtNetwork>
#include <QCoreApplication>
#include <QtGui>

#include "nullfxp.h"

#include "utils.h"
#include "localview.h"
#include "remoteview.h"
#include "progressdialog.h"
#include "connectingstatusdialog.h"
#include "quickconnectinfodialog.h"
#include "remotehostconnectthread.h"
#include "forwardconnectdaemon.h"

#include "synchronizeoptiondialog.h"
#include "synchronizewindow.h"

void NullFXP::slot_forward_connect(bool show)
{
    //qDebug() <<__FUNCTION__<<": "<<__LINE__<<":"<< __FILE__;
    if (fcd == 0) {
        fcd = new ForwardConnectDaemon(this);
        fcd->setObjectName("out");
    }
    //fcd->show();
    if (fcd->objectName() == "out" && show) {
        this->statusBar()->addPermanentWidget(fcd);
        fcd->setObjectName("in");
        if(!fcd->isVisible()) fcd->show();
    }
    if (fcd->objectName() == "in" && !show) {
        this->statusBar()->removeWidget(fcd);
        fcd->setObjectName("out");
    }
}

void NullFXP::slot_synchronize_file()
{
    //qDebug() <<__FUNCTION__<<": "<<__LINE__<<":"<< __FILE__;
    //QMessageBox::warning(this,tr("Infomation:"),tr("This feather will coming soon.") );

    SynchronizeOptionDialog *sync_dlg = new SynchronizeOptionDialog(this);
    sync_dlg->show();
}

void NullFXP::slot_set_mdi_area_background()
{
    q_debug()<<""; // #CDE8D0
    // QColor color(165, 165, 165);
    QColor color("#CDE8D0");
    QBrush brush = this->mdiArea->background();
    brush.setColor(color);
    QPixmap logo(":/icons/nullget-1.png");
    logo.scaled(300, 400);
    brush.setTexture(logo);

    QTransform trform;
    trform.rotate(60);
    // trform.scale(2.2, 2.8);
    trform.shear(2.5, 3.5);
    brush.setTransform(trform);

    this->mdiArea->setBackground(brush);

    // i dont want to set entie background, but only a little rect
}
