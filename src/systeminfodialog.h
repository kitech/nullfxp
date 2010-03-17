// systeminfodialog.h --- 
// 
// Author: liuguangzhao
// Copyright (C) 2007-2010 liuguangzhao@users.sf.net
// URL: 
// Created: 2010-03-17 08:40:29 +0800
// Version: $Id$
// 

#ifndef _SYSTEMINFODIALOG_H_
#define _SYSTEMINFODIALOG_H_

#include "ui_systeminfodialog.h"

class SystemInfoDialog : public QDialog
{
    Q_OBJECT;
public:
    SystemInfoDialog(QWidget *parent = 0, Qt::WindowFlags f = 0);
    ~SystemInfoDialog();
    
    void setSystemInfo(QString info);

private:
    Ui::SystemInfoDialog uiwin;
};

#endif /* _SYSTEMINFODIALOG_H_ */
