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
#ifndef FORWARDCONNECTDAEMON_H
#define FORWARDCONNECTDAEMON_H

#include <QtCore>
#include <QtGui>
#include <QWidget>

#include "ui_forwardconnectdaemon.h"

/**
	@author liuguangzhao <liuguangzhao@users.sourceforge.net>
*/
class ForwardConnectDaemon : public QWidget
{
Q_OBJECT
public:
    ForwardConnectDaemon(QWidget *parent = 0);

    ~ForwardConnectDaemon();
    private slots:
        void slot_custom_ctx_menu(const QPoint & pos);
        
    private:
        Ui::ForwardConnectDaemon ui_fcd;
};

#endif
