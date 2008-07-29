// remotehostconnectingstatusdialog.cpp --- 
// 
// Filename: remotehostconnectingstatusdialog.cpp
// Description: 
// Author: 刘光照<liuguangzhao@users.sf.net>
// Maintainer: 
// Copyright (C) 2007-2008 liuguangzhao <liuguangzhao@users.sf.net>
// http://www.qtchina.net
// http://nullget.sourceforge.net
// Created: 二  7月 22 21:18:55 2008 (CST)
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


#include <QtCore>
#include <QtGui>

#include "remotehostconnectingstatusdialog.h"

RemoteHostConnectingStatusDialog::RemoteHostConnectingStatusDialog ( QString user_name,QString host_name,QWidget* parent, Qt::WindowFlags f ) : QDialog ( parent, f )
{
	this->user_name= user_name;
	this->host_name=host_name;
    
    connect_status_dialog.setupUi(this);
    
    this->connect_status_dialog.progressBar->setValue(0);
    this->connect_status_dialog.lineEdit->setText(QString("%1@%2").arg(user_name).arg(host_name));
    QObject::connect(&this->timer,SIGNAL(timeout()),
                      this,SLOT(slot_time_out()) ) ;
    
    timer.setInterval(100);
    timer.start();
    
}


RemoteHostConnectingStatusDialog::~RemoteHostConnectingStatusDialog()
{
  this->stop_progress_bar();
}

void RemoteHostConnectingStatusDialog::slot_time_out()
{
    if(this->connect_status_dialog.progressBar->value()==100)
    {
        this->connect_status_dialog.progressBar->setValue(0);
    }
    else
    {
        this->connect_status_dialog.progressBar->setValue(this->connect_status_dialog.progressBar->value()+1);
    }
}

void  RemoteHostConnectingStatusDialog::slot_connect_state_changed(QString state_desc )
{
    this->connect_status_dialog.lineEdit_2->setText( state_desc ) ;
    qDebug()<<state_desc;
}

void RemoteHostConnectingStatusDialog::closeEvent ( QCloseEvent * event ) 
{
    emit cancel_connect();
    event->ignore();
    this->setVisible(false);
}
void RemoteHostConnectingStatusDialog::stop_progress_bar()
{
  timer.stop();
  this->connect_status_dialog.progressBar->setValue(100);
}

