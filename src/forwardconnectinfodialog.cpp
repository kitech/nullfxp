// forwardconnectinfodialog.cpp --- 
// 
// Author: liuguangzhao
// Copyright (C) 2007-2012 liuguangzhao@users.sf.net
// URL: http://www.qtchina.net http://nullget.sourceforge.net
// Created: 2007-08-29 16:41:01 +0800
// Version: $Id$
// 

#include "ui_forwardconnectinfodialog.h"
#include "forwardconnectinfodialog.h"

ForwardConnectInfoDialog::ForwardConnectInfoDialog(QWidget *parent )
    : QDialog(parent)
    , uiw(new Ui::ForwardConnectInfoDialog())
{
    this->uiw->setupUi(this);
}


ForwardConnectInfoDialog::~ForwardConnectInfoDialog()
{
}

void ForwardConnectInfoDialog::get_forward_info(QString &host,  QString &user_name, QString &passwd, 
                                                QString &listen_port, QString &local_port)
{
    host = this->uiw->lineEdit->text();
    user_name = this->uiw->lineEdit_2->text();
    passwd = this->uiw->lineEdit_3->text();
    listen_port = this->uiw->lineEdit_4->text();
    local_port = this->uiw->lineEdit_5->text();
}

