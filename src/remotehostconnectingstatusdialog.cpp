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
    timer.stop();
    this->connect_status_dialog.progressBar->setValue(100);
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
}

void RemoteHostConnectingStatusDialog::closeEvent ( QCloseEvent * event ) 
{
    emit cancel_connect();
    event->ignore();
    this->setVisible(false);
}
