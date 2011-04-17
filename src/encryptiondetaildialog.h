// encryptiondetaildialog.h --- 
// 
// Author: liuguangzhao
// Copyright (C) 2007-2010 liuguangzhao@users.sf.net
// URL: http://www.qtchina.net http://nullget.sourceforge.net
// Created: 2008-08-05 21:26:14 +0800
// Version: $Id$
// 

#ifndef ENCRYPTIONDETAILDIALOG_H
#define ENCRYPTIONDETAILDIALOG_H

#include <QtCore>
#include <QtGui>

// #include "ui_encryptiondetaildialog.h"
namespace Ui {
    class EncryptionDetailDialog;
};

class EncryptionDetailDialog : public QDialog
{
    Q_OBJECT;
public:
    EncryptionDetailDialog(char **server_info, QWidget *parent = 0, Qt::WindowFlags f = 0);
    virtual ~EncryptionDetailDialog();

public slots:
    void why();

signals:

private:
    Ui::EncryptionDetailDialog *ui_dlg;
};

#endif

