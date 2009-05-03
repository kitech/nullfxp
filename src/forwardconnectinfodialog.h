/***************************************************************************
 *   Copyright (C) 2007 by liuguangzhao   *
 *   liuguangzhao@users.sf.net   *
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
#ifndef FORWARDCONNECTINFODIALOG_H
#define FORWARDCONNECTINFODIALOG_H

#include <QtCore>
#include <QtGui>

#include "ui_forwardconnectinfodialog.h"
/**
	@author liuguangzhao <liuguangzhao@users.sf.net>
*/
class ForwardConnectInfoDialog : public QDialog
{
Q_OBJECT
public:
    ForwardConnectInfoDialog(QWidget *parent = 0);

    ~ForwardConnectInfoDialog();
    void get_forward_info(QString &host, QString &user_name, QString &passwd, QString &listen_port, QString &local_port);
    
    private:
        Ui::ForwardConnectInfoDialog fcid;
};

#endif
