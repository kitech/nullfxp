/***************************************************************************
 *   Copyright (C) 2007 by liuguangzhao   *
 *   liuguangzhao@users.sourceforge.net   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

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
    qDebug() <<__FUNCTION__<<": "<<__LINE__<<":"<< __FILE__;
}

