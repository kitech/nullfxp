// remotehostquickconnectinfodialog.cpp --- 
// 
// Filename: remotehostquickconnectinfodialog.cpp
// Description: 
// Author: 刘光照<liuguangzhao@users.sf.net>
// Maintainer: 
// Copyright (C) 2007-2008 liuguangzhao <liuguangzhao@users.sf.net>
// http://www.qtchina.net
// http://nullget.sourceforge.net
// Created: 日  6月  8 11:28:59 2008 (CST)
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

#include  <QtGui>

#include "remotehostquickconnectinfodialog.h"

RemoteHostQuickConnectInfoDialog::RemoteHostQuickConnectInfoDialog(QWidget* parent, Qt::WindowFlags f): QDialog(parent, f)
{
    this->quick_connect_info_dialog.setupUi(this);
    QObject::connect(this->quick_connect_info_dialog.checkBox_3, SIGNAL(stateChanged(int)),
                     this, SLOT(slot_pubkey_checked(int)));
    QObject::connect(this->quick_connect_info_dialog.toolButton, SIGNAL(clicked()),
                     this, SLOT(slot_select_pubkey()));
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
    QString port = host["port"];
    
    this->quick_connect_info_dialog.lineEdit->setText(host_name);
    this->quick_connect_info_dialog.lineEdit_2->setText(port);
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

void RemoteHostQuickConnectInfoDialog::slot_pubkey_checked(int state)
{
    if(state == Qt::Checked || state == Qt::PartiallyChecked) {
        this->quick_connect_info_dialog.toolButton->setEnabled(true);
    }else{
        this->quick_connect_info_dialog.toolButton->setEnabled(false);
    }
}

void RemoteHostQuickConnectInfoDialog::slot_select_pubkey()
{
    QString path = QString::null;

    path = QFileDialog::getOpenFileName ( this, tr("Publickey Select Dialog"),".");
    qDebug()<<path;    
}




