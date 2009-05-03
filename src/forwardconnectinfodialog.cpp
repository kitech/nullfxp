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
#include "forwardconnectinfodialog.h"

ForwardConnectInfoDialog::ForwardConnectInfoDialog(QWidget *parent )
    :QDialog(parent)
{
    this->fcid.setupUi(this);
}


ForwardConnectInfoDialog::~ForwardConnectInfoDialog()
{
}

void ForwardConnectInfoDialog::get_forward_info(QString &host,  QString &user_name, QString &passwd, QString &listen_port, QString &local_port)
{
    host = this->fcid.lineEdit->text();
    user_name = this->fcid.lineEdit_2->text();
    passwd = this->fcid.lineEdit_3->text();
    listen_port = this->fcid.lineEdit_4->text();
    local_port = this->fcid.lineEdit_5->text();
}

