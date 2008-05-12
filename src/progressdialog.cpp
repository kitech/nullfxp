// progressdialog.cpp --- 
// 
// Filename: progressdialog.cpp
// Description: 
// Author: liuguangzhao
// Maintainer: 
// Copyright (C) 2007-2008 liuguangzhao <liuguangzhao@users.sourceforge.net>
// http://www.qtchina.net
// http://nullget.sourceforge.net
// Created: 二  5月  6 21:59:14 2008 (CST)
// Version: 
// Last-Updated: 
//           By: 
//     Update #: 0
// URL: 
// Keywords: 
// Compatibility: 
// 
// 

// Commentary: 
// 
// 
// 
// 

// Change log:
// 
// 
// 


#include <cassert>
#include <stdint.h>

#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/param.h>

#ifdef HAVE_SYS_STAT_H
# include <sys/stat.h>
#endif
#ifdef HAVE_SYS_TIME_H
# include <sys/time.h>
#endif

#ifdef WIN32
#define jjjjjjjjj
#else
#include <sys/uio.h>
#endif

#include "globaloption.h"
#include "progressdialog.h"
#include "fileexistaskdialog.h"

//#define REMOTE_CODEC "UTF-8"

ProgressDialog::ProgressDialog(QWidget *parent )
    : QWidget(parent )
{
    this->ui_progress_dialog.setupUi(this);
    this->setObjectName("pv");
    //////////
    this->sftp_transfer_thread = new TransferThread();
    
    QObject::connect(this->sftp_transfer_thread,SIGNAL(finished()),
            this,SLOT(slot_transfer_thread_finished()) );
    QObject::connect(this->sftp_transfer_thread,SIGNAL(transfer_percent_changed(int,int ,int )),
                     this,SLOT(slot_set_transfer_percent(int,int,int)) );
    QObject::connect(this->sftp_transfer_thread,SIGNAL(transfer_new_file_started(QString)),
                     this,SLOT(slot_new_file_transfer_started(QString)));
    QObject::connect(this->ui_progress_dialog.pushButton,SIGNAL( clicked()),
                     this,SLOT(slot_cancel_button_clicked()));
    QObject::connect( this->sftp_transfer_thread,SIGNAL(transfer_got_file_size(int)),
                      this,SLOT(slot_transfer_got_file_size(int )) );
    QObject::connect(this->sftp_transfer_thread,SIGNAL(transfer_log(QString)),
                     this,SLOT(slot_transfer_log(QString)) );

    QObject::connect(this->sftp_transfer_thread, SIGNAL(dest_file_exists(QString, QString, QString, QString, QString, QString)),
		     this,SLOT(slot_dest_file_exists(QString,QString, QString, QString, QString, QString)));
    
    this->first_show = 1 ;
    this->ui_progress_dialog.progressBar->setValue(0);
    this->total_files_size = 0 ;
    this->total_files_count = 0 ;
    this->abtained_files_size = 0 ;
    this->abtained_files_count = 0 ;
}


ProgressDialog::~ProgressDialog()
{
    delete this->sftp_transfer_thread;
}

// void ProgressDialog::set_remote_connection(void* ssh2_sess  )
// {
//     this->sftp_transfer_thread->set_remote_connection( ssh2_sess  );
// }

void ProgressDialog::set_transfer_info(/*int type,*/QStringList local_file_names,QStringList remote_file_names  ) 
{
    QString local_file_name ;
    QString remote_file_name ;
    QString tmp_str ;
    //this->transfer_type = type ;
    
    this->local_file_names = local_file_names;
    this->remote_file_names = remote_file_names ;

    this->sftp_transfer_thread->set_transfer_info(/*type,*/local_file_names,remote_file_names  );
    
//     if(type == TransferThread::TRANSFER_PUT)
//     {
//         assert( remote_file_names.count() ==1);
//         QString remote_full_path ;
//         remote_file_name = remote_file_names.at(0);
//         
//         this->ui_progress_dialog.lineEdit->setText(tr("Uploading..."));
//         this->ui_progress_dialog.comboBox->clear();
//         this->ui_progress_dialog.comboBox_2->clear();
//         this->ui_progress_dialog.comboBox_2->addItem(remote_file_name);
//         
//         for(int i = 0 ; i < local_file_names.count() ; i ++)
//         {
//             local_file_name = local_file_names.at(i);
//             remote_full_path = remote_file_name + "/"
//                     + local_file_name.split ( "/" ).at ( local_file_name.split ( "/" ).count()-1 ) ;
//             this->ui_progress_dialog.comboBox_2->addItem( local_file_name );
// 
//         }
//     }
//     else if( type == TransferThread::TRANSFER_GET)
//     {
//         assert( local_file_names.count() == 1 ) ;
//         local_file_name = local_file_names.at(0);
//         
        QString local_full_path ;
//         this->ui_progress_dialog.lineEdit->setText(tr("Downloading..."));
        this->ui_progress_dialog.comboBox->clear();
        this->ui_progress_dialog.comboBox_2->clear();
//         this->ui_progress_dialog.comboBox_2->addItem( local_file_name );
//         
        for( int i = 0 ; i < remote_file_names.count() ; i ++ )
        {
            remote_file_name = remote_file_names.at(i);
            local_full_path = local_file_name + "/"
                + remote_file_name.split ( "/" ).at ( remote_file_name.split ( "/" ).count()-1 ) ;
            //fixed: 当用户名或者密码中包含问号或者#号的时候 QUrl 类就处理不了了。 传递过来的密码都是urlencoded
			tmp_str = QUrl(remote_file_name).path() ;
			if( tmp_str.at(2) == ':'){
				//assert it win32	"/G:/path/to/file.zip"
				this->ui_progress_dialog.comboBox_2->addItem( QUrl(remote_file_name).path().right(QUrl(remote_file_name).path().length()-1) );
			}else{
				this->ui_progress_dialog.comboBox_2->addItem( QUrl(remote_file_name).path() );
			}
            this->ui_progress_dialog.lineEdit->setText(QUrl(local_file_names.at(0)).scheme()+"-->"+QUrl(remote_file_name).scheme());
        }
//     }
//     else
//     {
//         assert(1==2);
//     }
}

void ProgressDialog::slot_set_transfer_percent(int percent  , int total_transfered,int transfer_delta )
{
    //qDebug() <<__FUNCTION__<<": "<<__LINE__<<":"<< __FILE__; 
    this->ui_progress_dialog.progressBar->setValue(percent);
    this->ui_progress_dialog.lineEdit_5->setText(QString("%1").arg(total_transfered));
    this->abtained_files_size += transfer_delta ;//TODO no right logic
    if( percent == 100 )
    {
        this->abtained_files_count += 1 ;
    }
    if(!start_time.isValid()){
        start_time = QDateTime::currentDateTime();
        transfer_speed = 0 ;
    }else{
        end_time = QDateTime::currentDateTime();
        if( (end_time.toTime_t()-start_time.toTime_t()) !=0 )
            transfer_speed = this->abtained_files_size/(end_time.toTime_t()-start_time.toTime_t());
        else
            transfer_speed = this->abtained_files_size/1;
        
        transfer_speed /= 1024 ;
    }
    this->update_transfer_state() ;
}

void ProgressDialog::slot_transfer_thread_finished() 
{
    qDebug() <<__FUNCTION__<<": "<<__LINE__<<":"<< __FILE__; 

    
    //this->done(QDialog::Accepted);
    int error_code = this->sftp_transfer_thread->get_error_code();
    
    emit this->transfer_finished(error_code);
    
}

void ProgressDialog::exec()
{

    qDebug() <<__FUNCTION__<<": "<<__LINE__<<":"<< __FILE__; 
    if(this->first_show)
    {
        this->first_show = 0 ;
        this->sftp_transfer_thread->start(); 
    }
    //QDialog::exec();
    
}
void ProgressDialog::show () 
{
    qDebug() <<__FUNCTION__<<": "<<__LINE__<<":"<< __FILE__; 
    if(this->first_show)
    {
        this->first_show = 0 ;
        this->sftp_transfer_thread->start(); 
    }
    QWidget::show(); 
}

void ProgressDialog::slot_new_file_transfer_started(QString new_file_name)
{
    
    QString u_new_file_name =  new_file_name  ;
    bool found = 0 ;
    for(int i = 0 ; i < this->ui_progress_dialog.comboBox->count() ; i ++ )
    {
        if( this->ui_progress_dialog.comboBox->itemText(i) == u_new_file_name )
        {
            this->ui_progress_dialog.comboBox->setCurrentIndex(i);
            found = 1 ;
            break ;
        }
    }
    if( found == 0 )
    {
        this->ui_progress_dialog.comboBox->addItem(u_new_file_name);
        this->ui_progress_dialog.comboBox->setCurrentIndex(this->ui_progress_dialog.comboBox->count()-1);
    }
    u_new_file_name = QString(tr("processing: %1")).arg(u_new_file_name);
    this->ui_progress_dialog.progressBar->setStatusTip( u_new_file_name );
    ////
    this->total_files_count ++ ;
    this->update_transfer_state();
    //this->setToolTip(u_new_file_name);
    this->ui_progress_dialog.lineEdit_4->setText(this->type(u_new_file_name));
}

void ProgressDialog::closeEvent ( QCloseEvent * event ) 
{
    int u_r = 0 ;
    
    //this->setVisible(false);
    u_r = QMessageBox::information(this,tr("Attemp to close this window?"),tr("Are you sure to stop the transfomition ?"),QMessageBox::Ok | QMessageBox::Cancel );
    if(u_r == QMessageBox::Ok){
        this->sftp_transfer_thread->set_user_cancel(true);
        event->ignore();
        this->setVisible(false);
        //emit this->transfer_finished(this->sftp_transfer_thread->get_error_code());
    }else{
        event->ignore();
    }
}

void ProgressDialog::slot_cancel_button_clicked()
{
    this->close() ;
}
void ProgressDialog::update_transfer_state()
{
    this->ui_progress_dialog.lineEdit_8->setText(QString("%1").arg(this->transfer_speed));
    this->ui_progress_dialog.lineEdit_9->setText(QString("%1").arg(this->abtained_files_count));
    this->ui_progress_dialog.lineEdit_10->setText(QString("%1").arg(this->total_files_count));
    this->ui_progress_dialog.lineEdit_11->setText(QString("%1").arg(this->abtained_files_size));
    this->ui_progress_dialog.lineEdit_12->setText(QString("%1").arg(this->total_files_size));
    if( this->total_files_size == 0 )
    {
        this->ui_progress_dialog.progressBar_2->setValue(100);
    }
    else
    {
        this->ui_progress_dialog.progressBar_2->setValue(100.0*((double)this->abtained_files_size/(double)this->total_files_size));
    }
}
void ProgressDialog::slot_transfer_got_file_size( int size )
{
    this->total_files_size += size ;
    this->ui_progress_dialog.lineEdit_13->setText(QString("%1").arg(size));
    this->update_transfer_state() ;
}
void ProgressDialog::slot_transfer_log(QString log)
{
    this->ui_progress_dialog.lineEdit_14->setText(log);
}

QString ProgressDialog::type(QString file_name)
{
    QFileInfo info(file_name);
    
    if (file_name == "/")
        return QApplication::translate("QFileDialog", "Drive");
//     if (info.isFile()) {
    if (1) {
        if (!info.suffix().isEmpty())
            return info.suffix() + QLatin1Char(' ') + QApplication::translate("QFileDialog", "File");
        return QApplication::translate("QFileDialog", "File");
    }

    if (info.isDir())
        return QApplication::translate("QFileDialog",
#ifdef Q_WS_WIN
                                       "File Folder", "Match Windows Explorer"
#else
                                       "Folder", "All other platforms"
#endif
            );
    // Windows   - "File Folder"
    // OS X      - "Folder"
    // Konqueror - "Folder"
    // Nautilus  - "folder"

    if (info.isSymLink())
        return QApplication::translate("QFileDialog",
#ifdef Q_OS_MAC
                                       "Alias", "Mac OS X Finder"
#else
                                       "Shortcut", "All other platforms"
#endif
            );
    // OS X      - "Alias"
    // Windows   - "Shortcut"
    // Konqueror - "Folder" or "TXT File" i.e. what it is pointing to
    // Nautilus  - "link to folder" or "link to object file", same as Konqueror

    return QApplication::translate("QFileDialog", "Unknown");
}

void ProgressDialog::slot_dest_file_exists(QString src_path, QString src_file_size, QString src_file_date, QString dest_path, QString dest_file_size, QString dest_file_date)
{
  qDebug()<<"Dest file exists: "<<dest_path<<". src path:"<<src_path;
  FileExistAskDialog * ask_dlg;
  int rv = 0;

  ask_dlg = new FileExistAskDialog(this);
  ask_dlg->set_files(src_path, src_file_size, src_file_date, dest_path, dest_file_size, dest_file_date);

  QObject::connect(ask_dlg, SIGNAL(acceptedOne(int)), this, SLOT(slot_ask_accepted(int)));

  rv = ask_dlg->exec();

  //delete ask_dlg;
  //this->sftp_transfer_thread->user_response_result(5);
}

void ProgressDialog::slot_ask_accepted(int which)
{
  FileExistAskDialog *ask_dlg = (FileExistAskDialog*)sender();
  delete ask_dlg;
  
  if(which >=TransferThread::OW_CANCEL && which <=TransferThread::OW_NO_ALL)
    this->sftp_transfer_thread->user_response_result(which);
  else
    qDebug()<<"No care response";
}
