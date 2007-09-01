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
#include "remotehostquickconnectinfodialog.h"

RemoteHostQuickConnectInfoDialog::RemoteHostQuickConnectInfoDialog(QWidget* parent, Qt::WindowFlags f): QDialog(parent, f)
{
    this->quick_connect_info_dialog.setupUi(this);
    QObject::connect(this->quick_connect_info_dialog.comboBox_3,SIGNAL( currentIndexChanged(int)),
                     this,SLOT(slot_test_remote_host_changed(int)) );
}


RemoteHostQuickConnectInfoDialog::~RemoteHostQuickConnectInfoDialog()
{
}

QString RemoteHostQuickConnectInfoDialog::get_user_name ()
{
    return this->quick_connect_info_dialog.lineEdit_3->text();
}

QString RemoteHostQuickConnectInfoDialog::get_host_name ()
{
    return this->quick_connect_info_dialog.lineEdit->text();
}

QString RemoteHostQuickConnectInfoDialog::get_password()
{
    return this->quick_connect_info_dialog.lineEdit_4->text();
}

void RemoteHostQuickConnectInfoDialog::slot_test_remote_host_changed(int value)
{
    qDebug() <<__FUNCTION__<<": "<<__LINE__<<":"<< __FILE__;
    switch(value)
    {
         
        case 0:
            this->quick_connect_info_dialog.lineEdit->setText("localhost");
            this->quick_connect_info_dialog.lineEdit_3->setText("root");
            break;
        case 1:
            this->quick_connect_info_dialog.lineEdit->setText("shell.sourceforge.net");
            this->quick_connect_info_dialog.lineEdit_3->setText("liuguangzhao");
            break;
        case 2:
            this->quick_connect_info_dialog.lineEdit->setText("www.bjdazhaimen.com");
            this->quick_connect_info_dialog.lineEdit_3->setText("root");
            
            break;
        case 3:
            this->quick_connect_info_dialog.lineEdit->setText("localhost");
            this->quick_connect_info_dialog.lineEdit_3->setText("gzl");

            break;
        default:
            break;
    }            
}
    


