// sshproxy.cpp --- 
// 
// Author: liuguangzhao
// Copyright (C) 2007-2012 liuguangzhao@users.sf.net
// URL: 
// Created: 2011-07-19 17:25:48 +0000
// Version: $Id$
// 

#include "simplelog.h"

#include "ui_sshproxy.h"
#include "sshproxy.h"


SSHProxy::SSHProxy(QWidget *parent)
    :QDialog(parent)
    ,uiw(new Ui::SSHProxy())
{
    this->uiw->setupUi(this);

    QObject::connect(this->uiw->pushButton_2, SIGNAL(clicked()), this, SLOT(slot_start()));
}

SSHProxy::~SSHProxy()
{
}

void SSHProxy::slot_start()
{
    qLogx()<<"";
    unsigned short lsn_port;
    QString ssh_sess_name;
}

