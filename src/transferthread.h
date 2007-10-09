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
#ifndef TRANSFERTHREAD_H
#define TRANSFERTHREAD_H

#include <vector>
#include <map>

#include <QtCore>

#include <QThread>

#include "libssh2.h"
#include "libssh2_sftp.h"

/**
	@author liuguangzhao <gzl@localhost>
*/
class TransferThread : public QThread
{
Q_OBJECT
public:
    
    enum { TRANSFER_GET,TRANSFER_PUT };
    
    TransferThread(QObject *parent = 0);

    ~TransferThread();

    void run();
    
    //void set_remote_connection( void* ssh2_sess  );
    
    //说明，在上传的时候local_file_names.count()可以大于1个，而remote_file_names.count()必须等于1
    //在下载的时候：local_file_names.count()必须等于1,而remote_file_names.count()可以大于1个
    //type 可以是 TANSFER_GET,TRANSFER_PUT
    void set_transfer_info(int type,QStringList local_file_names,QStringList remote_file_names ) ;
    
    int do_upload ( QString local_path, QString remote_path, int pflag );
    int  do_download ( QString remote_path, QString local_path,   int pflag )   ;
    int do_nrsftp_exchange( QString src_url , QString dest_path );
    
    int   get_error_code () { return this->error_code ;} 
    
    private :
        int remote_is_dir( LIBSSH2_SFTP * ssh2_sftp, QString path );
        int remote_is_reg( LIBSSH2_SFTP * ssh2_sftp, QString path ); 
        int fxp_do_ls_dir (LIBSSH2_SFTP * ssh2_sftp, QString parent_path  , QVector<QMap<char, QString> > & fileinfos    );
          
    signals:
        void  transfer_percent_changed( int percent , int total_transfered ,int transfer_delta );
        void  transfer_new_file_started(QString new_file_name);
        void  transfer_got_file_size( int size );
        
    private:
        
        LIBSSH2_SESSION * dest_ssh2_sess;
        LIBSSH2_SFTP * dest_ssh2_sftp;
        int dest_ssh2_sock ;
        
        LIBSSH2_SESSION * src_ssh2_sess ;
        LIBSSH2_SFTP * src_ssh2_sftp ;
        int src_ssh2_sock ;
        
        int transfer_type ;

        QStringList local_file_names ;
        QStringList remote_file_names;

        uint64_t total_file_size ;
        uint64_t total_transfered_file_length ; 
        uint32_t total_file_count ;
        uint32_t total_transfered_file_count ;
        uint64_t current_file_size ;
        uint64_t current_file_transfered_length ;
        QString  current_local_file_name;
        QString  current_local_file_type;
        QString  current_remote_file_name;
        QString  current_remote_file_type;
        //local_file_name     local_file_type   remote_file_name  remote_file_type 
        //std::vector< std::pair< std::pair<std::string , std::string>, std::pair<std::string,std::string> > > transfer_ready_queue ;
        //std::vector< std::pair< std::pair<std::string , std::string>, std::pair<std::string,std::string> > > transfer_done_queue ;
        //std::vector< std::pair< std::pair<std::string , std::string>, std::pair<std::string,std::string> > > transfer_error_queue ;  
		QVector<QPair<QPair<QString,QString> , QPair<QString,QString> > > transfer_ready_queue;
        QVector<QPair<QPair<QString,QString> , QPair<QString,QString> > > transfer_done_queue;
		QVector<QPair<QPair<QString,QString> , QPair<QString,QString> > > transfer_error_queue;
        //
        int error_code ;
        
};

#endif
