/* progressdialog.h --- 
 * 
 * Filename: progressdialog.h
 * Description: 
 * Author: liuguangzhao
 * Maintainer: 
 * Copyright (C) 2007-2008 liuguangzhao <liuguangzhao@users.sourceforge.net>
 * http://www.qtchina.net
 * http://nullget.sourceforge.net
 * Created: 二  5月  6 21:59:33 2008 (CST)
 * Version: 
 * Last-Updated: 一  5月 12 21:22:09 2008 (CST)
 *           By: liuguangzhao
 *     Update #: 1
 * URL: 
 * Keywords: 
 * Compatibility: 
 * 
 */

/* Commentary: 
 * 
 * 
 * 
 */

/* Change log:
 * 
 * 
 */


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
    
    public slots:
        void slot_set_transfer_percent(int percent , int total_transfered ,int transfer_delta );
        void slot_transfer_thread_finished() ;
        void slot_new_file_transfer_started(QString new_file_name);
        
        void exec ();
        void show () ;
        void slot_cancel_button_clicked();
        void slot_transfer_got_file_size( int size );
        void slot_transfer_log(QString log);
	void slot_dest_file_exists(QString src_path, QString src_file_size, QString src_file_date,QString dest_path, QString dest_file_size, QString dest_file_date);
	void slot_ask_accepted(int which);

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
        int  transfer_speed ;
        QDateTime start_time;
        QDateTime end_time ;
        
    private:    //UI element
        
        Ui::ProgressDialog ui_progress_dialog; 
        
    private:
        void update_transfer_state();
        QString type(QString file_name);
    protected:
        void closeEvent ( QCloseEvent * event ) ;
};

#endif
