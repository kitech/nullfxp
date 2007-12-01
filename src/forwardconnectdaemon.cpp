/***************************************************************************
 *   Copyright (C) 2007 by liuguangzhao   *
 *   liuguangzhao@users.sourceforge.net   *
 *
 *   http://www.qtchina.net                                                *
 *   http://nullget.sourceforge.net                                        *
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
#include "forwardconnectdaemon.h"

ForwardConnectDaemon::ForwardConnectDaemon(QWidget *parent)
 : QWidget(parent)
{
    this->ui_fcd.setupUi(this);
    
    QObject::connect ( this,SIGNAL ( customContextMenuRequested ( const QPoint & ) ),
                       this , SLOT ( slot_custom_ctx_menu ( const QPoint & ) ) );
    QObject::connect ( this->ui_fcd.comboBox,SIGNAL ( customContextMenuRequested ( const QPoint & ) ),
                       this , SLOT ( slot_custom_ctx_menu ( const QPoint & ) ) );
    QObject::connect ( this->ui_fcd.toolButton,SIGNAL ( customContextMenuRequested ( const QPoint & ) ),
                       this , SLOT ( slot_custom_ctx_menu ( const QPoint & ) ) );
}


ForwardConnectDaemon::~ForwardConnectDaemon()
{
}

void ForwardConnectDaemon::slot_custom_ctx_menu(const QPoint & pos)
{
    qDebug() <<__FUNCTION__<<": "<<__LINE__<<":"<< __FILE__;
}


