// systeminfodialog.cpp --- 
// 
// Author: liuguangzhao
// Copyright (C) 2007-2012 liuguangzhao@users.sf.net
// URL: 
// Created: 2010-03-17 08:40:12 +0800
// Version: $Id$
// 

#include <QtCore>

#include "systeminfodialog.h"
#include "ui_systeminfodialog.h"


SystemInfoDialog::SystemInfoDialog(QWidget *parent, Qt::WindowFlags f)
    : QDialog(parent, f)
    , uiw(new Ui::SystemInfoDialog())
{
    this->uiw->setupUi(this);
}

SystemInfoDialog::~SystemInfoDialog()
{

}

void SystemInfoDialog::setSystemInfo(QString info)
{
    QString hinfo = QString("<center><h3>System infomation</h3></center><BR>  <B>%1</B>").arg(info);
    this->uiw->label->setText(hinfo);
    this->uiw->label->setToolTip(info);

#ifdef Q_OS_MAC
    this->uiw->label_2->setPixmap(QPixmap(qApp->applicationDirPath() + "/icons/os/osx.png").scaled(90, 90));
#else
#ifdef Q_OS_WIN
    this->uiw->label_2->setPixmap(QPixmap(qApp->applicationDirPath() + "/icons/os/windows.png").scaled(90, 90));
#else
    this->uiw->label_2->setPixmap(QPixmap(qApp->applicationDirPath() + "/icons/os/tux.png").scaled(90, 90));
#endif
#endif
}
