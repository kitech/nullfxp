// connectingstatusdialog.h --- 
// 
// Author: liuguangzhao
// Copyright (C) 2007-2010 liuguangzhao@users.sf.net
// URL: http://www.qtchina.net http://nullget.sourceforge.net
// Created: 2008-07-22 21:18:30 +0800
// Version: $Id: connectingstatusdialog.h 536 2009-10-08 15:38:00Z liuguangzhao $
// 

#ifndef CONNECTINGSTATUSDIALOG_H
#define CONNECTINGSTATUSDIALOG_H

#include <QtCore>
#include <QDialog>

#include "ui_connectingstatusdialog.h"

/**
 * 连接到服务器的状态进度提示对话框类
 */
class ConnectingStatusDialog : public QDialog
{
    Q_OBJECT;
public:
    ConnectingStatusDialog(QString user_name, QString host_name, QString port, QWidget *parent, Qt::WindowFlags f);

    ~ConnectingStatusDialog();
    
public slots:
    void slot_connect_state_changed(QString state_desc);
	void stop_progress_bar();

private slots:
    void slot_time_out();
signals:
    void cancel_connect();
        
private:
    QString host_name;
    QString user_name;
    QString port;
    QTimer  timer;
        
    Ui::ConnectingStatusDialog connect_status_dialog;

protected:
    virtual void closeEvent(QCloseEvent *event);
};

#endif
