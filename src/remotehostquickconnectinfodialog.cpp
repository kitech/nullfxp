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
short  RemoteHostQuickConnectInfoDialog::get_port()
{
    QString str_port = this->quick_connect_info_dialog.lineEdit_2->text();
    int port = str_port.toShort();
    
    return port;
}

void RemoteHostQuickConnectInfoDialog::set_active_host(QMap<QString,QString> host)
{
  QString show_name = host["show_name"];
  QString host_name = host["host_name"];
  QString user_name = host["user_name"];
  QString password = host["password"];

  this->quick_connect_info_dialog.lineEdit->setText(host_name);
  this->quick_connect_info_dialog.lineEdit_3->setText(user_name);

  this->quick_connect_info_dialog.lineEdit_4->setFocus();
  this->quick_connect_info_dialog.lineEdit_4->setText(password);

  this->quick_connect_info_dialog.groupBox->setTitle(QString(tr("Host Infomation: %1")).arg(show_name));
  this->show_name = show_name;
}
QMap<QString,QString> RemoteHostQuickConnectInfoDialog::get_host_map()
{
  QMap<QString,QString> host;

  host["show_name"] = this->show_name;
  host["host_name"] = this->quick_connect_info_dialog.lineEdit->text();
  host["user_name"] = this->quick_connect_info_dialog.lineEdit_3->text();
  host["password"] = this->quick_connect_info_dialog.lineEdit_4->text();
  host["port"] = this->quick_connect_info_dialog.lineEdit_2->text();

  return host;
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
            this->quick_connect_info_dialog.lineEdit_3->setText("webroot");
            
            break;
        case 3:
            this->quick_connect_info_dialog.lineEdit->setText("localhost");
            this->quick_connect_info_dialog.lineEdit_3->setText("gzl");

            break;
        case 4:
            this->quick_connect_info_dialog.lineEdit->setText("192.168.1.6");
            this->quick_connect_info_dialog.lineEdit_3->setText("gzl");

            break;
        default:
            break;
    }            
}
    


