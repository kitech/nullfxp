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
#include <sys/uio.h>

#include "includes.h"
#include "xmalloc.h"
#include "atomicio.h"

#include "progressmeter.h"
#include "sftp-common.h"
#include "sys-queue.h"

#include "sftp.h"
#include "sftp-operation.h"
#include "sftp-client.h"
#include "sftp-wrapper.h"


#include "progressdialog.h"

ProgressDialog::ProgressDialog(QWidget *parent)
 : QDialog(parent)
{
    this->ui_progress_dialog.setupUi(this);
    
    this->sftp_transfer_thread = new TransferThread();
    
    QObject::connect(this->sftp_transfer_thread,SIGNAL(finished()),
            this,SLOT(slot_transfer_thread_finished()) );
    QObject::connect(this->sftp_transfer_thread,SIGNAL(transfer_percent_changed(int )),
                     this,SLOT(slot_set_transfer_percent(int)) );
    
    this->first_show = 1 ;
    this->ui_progress_dialog.progressBar->setValue(0);
}


ProgressDialog::~ProgressDialog()
{
    delete this->sftp_transfer_thread;
}

void ProgressDialog::set_remote_connection(struct sftp_conn * connection)
{
    this->sftp_connection = connection ;
    this->sftp_transfer_thread->set_remote_connection(connection);
}

void ProgressDialog::set_transfer_info(int type,QString local_file_name,QString local_file_type,QString remote_file_name ,QString remote_file_type  ) 
{
    this->transfer_type = type ;
       
    this->local_file_name = local_file_name;
    this->local_file_type = local_file_type ;
    this->remote_file_name = remote_file_name ;
    this->remote_file_type = remote_file_type ;
    this->sftp_transfer_thread->set_transfer_info(type,local_file_name,local_file_type,remote_file_name , remote_file_type );
    
    
    if(type == TransferThread::TRANSFER_PUT)
    {
        QString remote_full_path = this->remote_file_name + "/"
                + this->local_file_name.split ( "/" ).at ( this->local_file_name.split ( "/" ).count()-1 ) ;
        
        this->ui_progress_dialog.lineEdit->setText("Uploading...");
        if( is_dir(this->local_file_name.toAscii().data())
            && remote_is_dir(this->sftp_connection,this->remote_file_name.toAscii().data()))
        {
            this->ui_progress_dialog.lineEdit_2->setText(local_file_name);
            this->ui_progress_dialog.lineEdit_3->setText(remote_file_name);    
        }
        else
        {
            this->ui_progress_dialog.lineEdit_2->setText(local_file_name);
            this->ui_progress_dialog.lineEdit_3->setText(remote_full_path);            
        }

    }
    else if( type == TransferThread::TRANSFER_GET)
    {
        QString local_full_path = this->local_file_name + "/"
                + this->remote_file_name.split ( "/" ).at ( this->remote_file_name.split ( "/" ).count()-1 ) ;
        this->ui_progress_dialog.lineEdit->setText("Downloading...");
        if( is_dir(this->local_file_name.toAscii().data())
            && remote_is_dir(this->sftp_connection,this->remote_file_name.toAscii().data()))
        {
            this->ui_progress_dialog.lineEdit_2->setText(remote_file_name);
            this->ui_progress_dialog.lineEdit_3->setText(local_file_name);  
        }
        else
        {
            this->ui_progress_dialog.lineEdit_2->setText(remote_file_name);
            this->ui_progress_dialog.lineEdit_3->setText(local_full_path);       
        }
    }
    else
    {
        assert(1==2);
    }
    
    
}


void ProgressDialog::slot_set_transfer_percent(int percent )
{
    //qDebug() <<__FUNCTION__<<": "<<__LINE__<<":"<< __FILE__; 
    
    this->ui_progress_dialog.progressBar->setValue(percent);
    
}

void ProgressDialog::slot_transfer_thread_finished() 
{
    qDebug() <<__FUNCTION__<<": "<<__LINE__<<":"<< __FILE__; 

    
    this->done(QDialog::Accepted);
    
    emit this->transfer_finished(0);
    
}

void ProgressDialog::exec()
{

    qDebug() <<__FUNCTION__<<": "<<__LINE__<<":"<< __FILE__; 
    if(this->first_show)
    {
        this->first_show = 0 ;
        this->sftp_transfer_thread->start(); 
    }
    QDialog::exec();
    
}




