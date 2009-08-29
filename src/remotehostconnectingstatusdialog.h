// remotehostconnectingstatusdialog.h --- 
// 
// Author: liuguangzhao
// Copyright (C) 2007-2010 liuguangzhao@users.sf.net
// URL: http://www.qtchina.net http://nullget.sourceforge.net
// Created: 2008-07-22 21:18:30 +0800
// Version: $Id$
// 

#ifndef REMOTEHOSTCONNECTINGSTATUSDIALOG_H
#define REMOTEHOSTCONNECTINGSTATUSDIALOG_H

#include <QtCore>
#include <QDialog>

#include "ui_remotehostconnectingstatusdialog.h"

/**
	@author liuguangzhao <gzl@localhost>
*/
class RemoteHostConnectingStatusDialog : public QDialog
{
Q_OBJECT
public:
    RemoteHostConnectingStatusDialog(QString user_name,QString host_name,QWidget* parent, Qt::WindowFlags f);

    ~RemoteHostConnectingStatusDialog();
    
    public slots:
        void  slot_connect_state_changed(QString state_desc );
	void stop_progress_bar();

    private slots:
        void slot_time_out();
    signals:
        void cancel_connect();
        
    private:
        QString host_name;
        QString user_name ;
            
        QTimer  timer ;
        
        Ui::RemoteHostConnectingStatusDialog connect_status_dialog;
    protected:
        virtual void closeEvent ( QCloseEvent * event ) ;
};

#endif
