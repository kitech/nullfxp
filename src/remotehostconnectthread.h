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
#ifndef REMOTEHOSTCONNECTTHREAD_H
#define REMOTEHOSTCONNECTTHREAD_H

#include <QtCore>

#include <QThread>


#include "sftp-operation.h"
#include "sftp-client.h"
#include "sftp-wrapper.h"

/**
	@author liuguangzhao <gzl@localhost>
*/
class RemoteHostConnectThread : public QThread
{
Q_OBJECT
public:
    RemoteHostConnectThread(QString user_name , QString password , QString host_name ,QObject* parent=0);

    ~RemoteHostConnectThread();

    virtual void run();
    
    void do_init() ;
    void diconnect_ssh_connection();
    
    private:
        std::string user_name;
        std::string password;
        std::string host_name ;
        int connect_status;
        struct sftp_conn * sftp_connection ;
        
        int child_pid   ;
        
    private slots:
        void slot_finished()   ;
        
    signals:
        void connect_finished( int status , struct sftp_conn* conn );
};

#endif
