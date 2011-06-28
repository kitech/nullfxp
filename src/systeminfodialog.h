// systeminfodialog.h --- 
// 
// Author: liuguangzhao
// Copyright (C) 2007-2012 liuguangzhao@users.sf.net
// URL: 
// Created: 2010-03-17 08:40:29 +0800
// Version: $Id$
// 

#ifndef _SYSTEMINFODIALOG_H_
#define _SYSTEMINFODIALOG_H_

#include <QtGui>

namespace Ui {
    class SystemInfoDialog;
};

class SystemInfoDialog : public QDialog
{
    Q_OBJECT;
public:
    SystemInfoDialog(QWidget *parent = 0, Qt::WindowFlags f = 0);
    virtual ~SystemInfoDialog();
    
    void setSystemInfo(QString info);

private:
    Ui::SystemInfoDialog *uiw;
};

#endif /* _SYSTEMINFODIALOG_H_ */
