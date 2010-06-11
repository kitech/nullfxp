// ftphostinfodialog.h --- 
// 
// Author: liuguangzhao
// Copyright (C) 2007-2010 liuguangzhao@users.sf.net
// URL: 
// Created: 2010-06-10 23:06:16 +0800
// Version: $Id$
// 

#ifndef _FTPHOSTINFODIALOG_H_
#define _FTPHOSTINFODIALOG_H_

#include <QtCore>
#include <QtGui>


#include "ui_ftphostinfodialog.h"
/**
 *
 * show something like, welcome mesg, host type.
 * supported command.
 */
class FTPHostInfoDialog : public QDialog
{
    Q_OBJECT;
public:
    FTPHostInfoDialog(QWidget *parent = 0);
    virtual ~FTPHostInfoDialog();

    void setHostType(QString type);
    void setWelcome(QString welcome);

private:
    Ui::FTPHostInfoDialog uiw;
    QGraphicsScene *mainScene;
};

#endif /* _FTPHOSTINFODIALOG_H_ */
