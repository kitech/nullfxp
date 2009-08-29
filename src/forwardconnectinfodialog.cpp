// forwardconnectinfodialog.cpp --- 
// 
// Author: liuguangzhao
// Copyright (C) 2007-2010 liuguangzhao@users.sf.net
// URL: http://www.qtchina.net http://nullget.sourceforge.net
// Created: 2007-08-29 16:41:01 +0800
// Version: $Id$
// 

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

