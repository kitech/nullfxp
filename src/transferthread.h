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



#include "sftp-client.h"

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
    
    void set_remote_connection(struct sftp_conn * connection);
    
    //type 可以是 TANSFER_GET,TRANSFER_PUT
    void set_transfer_info(int type,QString local_file_name,QString local_file_type,QString remote_file_name, QString remote_file_type ) ;
    
    int
            do_upload ( struct sftp_conn *conn, char *local_path, char *remote_path,
                        int pflag );
    int
            do_download ( struct sftp_conn *conn, char *remote_path, char *local_path,
                                          int pflag )   ;
     
    signals:
        void  transfer_percent_changed(int percent);
        
    private:
        
        struct sftp_conn * sftp_connection ;
        int transfer_type ;
        QString local_file_name ;
        QString local_file_type ;
        QString remote_file_name ;        
        QString remote_file_type ;
        
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
        std::vector< std::pair< std::pair<std::string , std::string>, std::pair<std::string,std::string> > > transfer_ready_queue ;
        std::vector< std::pair< std::pair<std::string , std::string>, std::pair<std::string,std::string> > > transfer_done_queue ;
        std::vector< std::pair< std::pair<std::string , std::string>, std::pair<std::string,std::string> > > transfer_error_queue ;        
};

#endif
