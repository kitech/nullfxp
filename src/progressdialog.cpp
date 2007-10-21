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
//#include "openbsd-compat/sys-queue.h"
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
            this->ui_progress_dialog.comboBox_2->addItem( QUrl(remote_file_name).path() );
    
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
}

void ProgressDialog::closeEvent ( QCloseEvent * event ) 
{
    event->ignore();
    //this->setVisible(false);
    QMessageBox::information(this,tr("Attemp to close this window?"),tr("Are you sure to stop the transfomition ?"),QMessageBox::Ok | QMessageBox::Cancel );
}

void ProgressDialog::slot_cancel_button_clicked()
{
    this->close() ;
}
void ProgressDialog::update_transfer_state()
{
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
        this->ui_progress_dialog.progressBar_2->setValue(100*this->abtained_files_size/this->total_files_size);
    }
}
void ProgressDialog::slot_transfer_got_file_size( int size )
{
    this->total_files_size += size ;
    this->ui_progress_dialog.lineEdit_13->setText(QString("%1").arg(size));
    this->update_transfer_state() ;
}

