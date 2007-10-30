/***************************************************************************
 *   Copyright (C) 2007 by liuguangzhao   *
 *   liuguangzhao@users.sourceforge.net   *
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

#include "transferthread.h"

#include "ui_progressdialog.h"

/**
	@author liuguangzhao <gzl@localhost>
*/
class ProgressDialog : public QWidget
{
Q_OBJECT
public:
    
    ProgressDialog(QWidget *parent = 0  );

    ~ProgressDialog();

    //void set_remote_connection(void* ssh2_sess );
    
    //type 可以是 TANSFER_GET,TRANSFER_PUT
    void set_transfer_info(/*int type,*/QStringList local_file_names,QStringList remote_file_names  ) ;
    
    //int get_transfer_type() { return this->transfer_type ; }
    
    public slots:
        void slot_set_transfer_percent(int percent , int total_transfered ,int transfer_delta );
        void slot_transfer_thread_finished() ;
        void slot_new_file_transfer_started(QString new_file_name);
        
        void exec ();
        void show () ;
        void slot_cancel_button_clicked();
        void slot_transfer_got_file_size( int size );
        void slot_transfer_log(QString log);
        
    signals:
        void transfer_finished(int status);
        
    private:

        //int transfer_type ;
        QStringList local_file_names ;
        QStringList remote_file_names ;

        TransferThread * sftp_transfer_thread ;
        bool   first_show ;
        
        quint64 total_files_size ;
        quint64 abtained_files_size ;
        int     total_files_count ;
        int     abtained_files_count ;
        
    private:    //UI element
        
        Ui::ProgressDialog ui_progress_dialog; 
        
    private:
        void update_transfer_state();
        QString type(QString file_name);
    protected:
        void closeEvent ( QCloseEvent * event ) ;
};

#endif
