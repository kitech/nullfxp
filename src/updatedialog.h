// updatedialog.h --- 
// 
// Author: liuguangzhao
// Copyright (C) 2007-2010 liuguangzhao@users.sf.net
// URL: http://www.qtchina.net http://nullget.sf.net
// Created: 2009-10-12 19:55:39 +0800
// Version: $Id$
// 

#ifndef _UPDATEDIALOG_H_
#define _UPDATEDIALOG_H_

#include <QDialog>
#include <QtNetwork>

namespace Ui {
    class UpdateDialog;
};

class UpdateDialog : public QDialog
{
    Q_OBJECT;
public:
    UpdateDialog(QWidget *parent = 0);
    virtual ~UpdateDialog();

private slots:
    void slotStartCheck();
    void slotCancelCheck();

    void slotDataReadProgress(int done, int total);
    void slotDone(bool error);
    void slotReadyRead(const QHttpResponseHeader &resp);

private:
    bool hasUpdate(QString lastestVersion);

private:
    Ui::UpdateDialog *uiw;
    bool inChecking;
    QHttp *http;
};


#endif /* _UPDATEDIALOG_H_ */
