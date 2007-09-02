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
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <errno.h>
#include <signal.h>
#include <wait.h>


#include "buffer.h"
#include "sftp.h"
//#include "log.h"
#include "xmalloc.h"
#include "defines.h"
#include "misc.h"


#include "glob.h"

#include "sftp-operation.h"

#include "localview.h"
#include "remoteview.h"
#include "progressdialog.h"



#include "remotehostconnectthread.h"

RemoteHostConnectThread::RemoteHostConnectThread(QString user_name , QString password , QString host_name ,QObject* parent): QThread(parent)
{
    this->user_name = user_name.toStdString();
    this->password = password.toStdString();
    this->host_name = host_name.toStdString();
    this->connect_status = 0;
    this->sftp_connection = 0 ;
    
    ////////////////
    QObject::connect(this,SIGNAL(finished()) ,this , SLOT(slot_finished()) );
    this->start();
}


RemoteHostConnectThread::~RemoteHostConnectThread()
{
}


void RemoteHostConnectThread::run()
{
    qDebug() <<__FUNCTION__<<": "<<__LINE__<<":"<< __FILE__;
    
    QString username ;
    QString password ;
    QString remoteaddr ;
    
    //QString program = "/home/gzl/openssh-4.6p1/ssh";
    QString program =  QCoreApplication::applicationDirPath () + "/plinker" ;
    
    
    int in , out ;
    this->sftp_connection = (struct sftp_conn*) xmalloc(sizeof(struct sftp_conn));
    memset(this->sftp_connection , 0 , sizeof(struct sftp_conn) );
    
    username = QString( this->user_name.c_str());
    password = QString( this->password.c_str()) ;
    remoteaddr = QString ( this->host_name.c_str()) ;
    
    arglist args ;

    memset ( &args, '\0', sizeof ( args ) );
    args.list = NULL;

    addargs ( &args, "%s", program.toAscii().data() );
    addargs ( &args, "-oForwardX11 no" );
    addargs ( &args, "-oForwardAgent no" );
    addargs ( &args, "-oPermitLocalCommand no" );
    addargs ( &args, "-oClearAllForwardings yes" );
    addargs ( &args, "-v" );
    addargs ( &args, "-v" );
    addargs ( &args, "-l%s",username.toAscii().data() );
    addargs ( &args, "-oProtocol %d",2 );
    addargs ( &args, "-y" );
    addargs ( &args, "%s",password.toAscii().data() );
    addargs ( &args, "-s" );
    addargs ( &args, "%s",remoteaddr.toAscii().data() );
    addargs ( &args, "sftp" );

    connect_to_server ( program .toAscii().data(),args.list,&in,&out , & this->child_pid  );
    qDebug() <<"Exec ret :" << in << " out: " << out << "child : ";

    this->sftp_connection->fd_in = in;
    this->sftp_connection->fd_out = out ;
	//theconn.transfer_buflen = 32768;
    this->sftp_connection->transfer_buflen = 1024;
    this->sftp_connection->num_requests = 0 ;
    this->sftp_connection->version = 0;
    this->sftp_connection->msg_id = 0 ;

    this->do_init();    
    
    //取得远程目录当前路径 pwd,一般就是home目录。
    char * pwd = 0 ;
    
    pwd = do_realpath(this->sftp_connection,".");
    
    assert( pwd != NULL );
    
    qDebug()<<" user default path is : "<< pwd ;
    
    user_home_path = std::string(pwd);
    
}

void RemoteHostConnectThread::slot_finished()
{
    emit this->connect_finished(this->connect_status,this->sftp_connection);
}

void RemoteHostConnectThread::do_init()
{
    qDebug() <<__FUNCTION__<<": "<<__LINE__<<":"<< __FILE__;
    int fd_in,  fd_out;
    u_int transfer_buflen=128, num_requests;
    fd_in = this->sftp_connection->fd_in;
    fd_out = this->sftp_connection->fd_out ;

    u_int type;
    int version;
    Buffer msg;
    struct sftp_conn *ret = this->sftp_connection ;

    buffer_init ( &msg );
    buffer_put_char ( &msg, SSH2_FXP_INIT );
    buffer_put_int ( &msg, SSH2_FILEXFER_VERSION );
    send_msg ( fd_out, &msg );

    buffer_clear ( &msg );

    ///////
    get_msg ( fd_in, &msg );

    /* Expecting a VERSION reply */
    if ( ( type = buffer_get_char ( &msg ) ) != SSH2_FXP_VERSION )
    {
        printf ( "Invalid packet back from SSH2_FXP_INIT (type %u)\n",
                type );
        buffer_free ( &msg );
		//return(NULL);
        return ;
    }
    version = buffer_get_int ( &msg );

    printf ( "Remote version: %d\n", version );

    printf ( "buffer_len: %d\n",buffer_len ( &msg ) );
    /* Check for extensions */
    while ( buffer_len ( &msg ) > 0 )
    {
        char *name = ( char* ) buffer_get_string ( &msg, NULL );
        char *value = ( char* ) buffer_get_string ( &msg, NULL );

        printf ( "Init extension: \"%s\"\n", name );
        xfree ( name );
        xfree ( value );
    }

    buffer_free ( &msg );

	//ret = xmalloc(sizeof(*ret));
    ret->fd_in = fd_in;
    ret->fd_out = fd_out;
    ret->transfer_buflen = transfer_buflen;
    ret->num_requests = num_requests;
    ret->version = version;
    ret->msg_id = 1;

    /* Some filexfer v.0 servers don't support large packets */
    if ( version == 0 )
        ret->transfer_buflen = MIN ( ret->transfer_buflen, 20480 );
}

void RemoteHostConnectThread::diconnect_ssh_connection()
{
    qDebug() <<__FUNCTION__<<": "<<__LINE__<<":"<< __FILE__;
    
    int ret = 0 ;
    if( this->child_pid > 0 )
    {
        ret = kill(this->child_pid,SIGHUP);
        qDebug() << "attemp to kill child process :" << this->child_pid <<" ret:"<<ret ;
        ret = kill(this->child_pid,SIGTERM);
        qDebug() << "attemp to kill child process :" << this->child_pid <<" ret:"<<ret ;
        ret = kill(this->child_pid,SIGABRT);
        qDebug() << "attemp to kill child process :" << this->child_pid <<" ret:"<<ret ;
        waitpid(this->child_pid, NULL, 0);
    }
}

std::string RemoteHostConnectThread::get_user_home_path () 
{
    return this->user_home_path ;
}

