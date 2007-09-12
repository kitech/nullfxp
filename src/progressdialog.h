/***************************************************************************
 *   Copyright (C) 2007 by liuguangzhao   *
 *   gzl@localhost   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
#ifndef PROGRESSDIALOG_H
#define PROGRESSDIALOG_H

#include <QtCore>
#include <QtGui>
#include <QDialog>

#include "sftp-client.h"

#include "transferthread.h"

#include "ui_progressdialog.h"

/**
	@author liuguangzhao <gzl@localhost>
*/
class ProgressDialog : public QDialog
{
Q_OBJECT
public:
    
    ProgressDialog(QWidget *parent = 0);

    ~ProgressDialog();

    void set_remote_connection(struct sftp_conn * connection);
    
    //type 可以是 TANSFER_GET,TRANSFER_PUT
    //void set_transfer_info(int type,QString local_file_name,QString local_file_type,QString remote_file_name ,QString remote_file_type ) ;
    void set_transfer_info(int type,QStringList local_file_names,QStringList remote_file_names  ) ;
    
    int get_transfer_type() { return this->transfer_type ; }
    
    public slots:
        void slot_set_transfer_percent(int percent );
        void slot_transfer_thread_finished() ;
        void slot_new_file_transfer_started(QString new_file_name);
        
        
        void exec ();
        
    signals:
        void transfer_finished(int status);
        
    private:
        struct sftp_conn * sftp_connection ;
        int transfer_type ;
        //QString local_file_name ;
        QStringList local_file_names ;
        //QString local_file_type ;
        //QString remote_file_name ;
        QStringList remote_file_names ;
        //QString remote_file_type ;
        
        //QTimer  * transfer_thread_finish_poll_timer ;
        TransferThread * sftp_transfer_thread ;
        bool   first_show ;
        
    private:    //UI element
        
        Ui::ProgressDialog ui_progress_dialog; 
        
};

#endif
