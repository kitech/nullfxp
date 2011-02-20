// connectingstatusdialog.cpp --- 
// 
// Author: liuguangzhao
// Copyright (C) 2007-2010 liuguangzhao@users.sf.net
// URL: http://www.qtchina.net http://nullget.sourceforge.net
// Created: 2008-07-22 21:18:55 +0800
// Version: $Id: connectingstatusdialog.cpp 536 2009-10-08 15:38:00Z liuguangzhao $
// 

#include <QtCore>
#include <QtGui>

#include "ui_connectingstatusdialog.h"

#include "connectingstatusdialog.h"

ConnectingStatusDialog::ConnectingStatusDialog(QString user_name, QString host_name, 
                                               QString port, QWidget *parent, Qt::WindowFlags f)
: QDialog(parent, f)
    , uiwin(new Ui::ConnectingStatusDialog())
{
	this->user_name = user_name;
	this->host_name = host_name;
    this->port = port;
    
    uiwin->setupUi(this);
    
    this->uiwin->progressBar->setValue(0);
    this->uiwin->lineEdit->setText(QString("%1@%2:%3")
                                   .arg(user_name).arg(host_name).arg(port));
    QObject::connect(&this->timer,SIGNAL(timeout()),
                     this, SLOT(slot_time_out()));
    
    timer.setInterval(100);
    timer.start();
}


ConnectingStatusDialog::~ConnectingStatusDialog()
{
    this->stop_progress_bar();
}

void ConnectingStatusDialog::slot_time_out()
{
    if (this->uiwin->progressBar->value()==100) {
        this->uiwin->progressBar->setValue(0);
    } else {
        this->uiwin->progressBar->setValue(this->uiwin->progressBar->value()+1);
    }
}

void  ConnectingStatusDialog::slot_connect_state_changed(QString state_desc)
{
    this->uiwin->lineEdit_2->setText(state_desc);
    this->uiwin->textBrowser->append(state_desc);
    qDebug()<<state_desc;
}

void ConnectingStatusDialog::closeEvent(QCloseEvent * event)
{
    emit cancel_connect();
    event->ignore();
    this->setVisible(false);
}

void ConnectingStatusDialog::stop_progress_bar()
{
    timer.stop();
    this->uiwin->progressBar->setValue(100);
}

