/***************************************************************************
 *   Copyright (C) 2007 by liuguangzhao   *
 *   gzl@localhost   *
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

int main(int argc, char *argv[])
{
    int app_exec_ret = -1;
      QApplication app(argc, argv);
      app.setApplicationVersion(NULLFXP_RELEASE);
      app.setOrganizationDomain(NULLFXP_HOMEPAGE);
      app.setOrganizationName("kitsoft");

      NullFXP nfxp ;
      nfxp.showNormal ();
      
      app_exec_ret = app.exec();
      if( app_exec_ret == 0)
          qDebug()<<"app exit normally.";
      else
          qDebug()<<"app exit with code:"<<app_exec_ret;
      return app_exec_ret;
}

