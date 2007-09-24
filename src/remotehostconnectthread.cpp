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
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>

// #include "sftp.h"
// 
// #include "sftp-operation.h"

#include "localview.h"
#include "remoteview.h"
#include "progressdialog.h"

#include "remotehostconnectthread.h"

#include "libssh2.h"
#include "libssh2_sftp.h"

//static char ssh2_user_name[60];
static char ssh2_password[60] ;

static void kbd_callback(const char *name, int name_len, 
                         const char *instruction, int instruction_len, int num_prompts,
                         const LIBSSH2_USERAUTH_KBDINT_PROMPT *prompts,
                         LIBSSH2_USERAUTH_KBDINT_RESPONSE *responses,
                         void **abstract)
{
    (void)name;
    (void)name_len;
    (void)instruction;
    (void)instruction_len;
    if (num_prompts == 1) {
        responses[0].text = strdup(ssh2_password);
        responses[0].length = strlen(ssh2_password);
    }
    (void)prompts;
    (void)abstract;
} /* kbd_callback */

RemoteHostConnectThread::RemoteHostConnectThread(QString user_name , QString password , QString host_name ,QObject* parent): QThread(parent)
{
    this->user_name = user_name.toStdString();
    this->password = password.toStdString();
    this->host_name = host_name.toStdString();
    this->connect_status = 0;
    
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
    
    //LIBSSH2_SESSION * ssh2_sess =0 ;    
    //LIBSSH2_SFTP * ssh2_sftp =0;
    int ret= 0;
    char home_path[PATH_MAX+1] = {0};
    char host_ipadd[60] = {0};
    
    //create socket 
    struct sockaddr_in serv_addr ;
    memset( & serv_addr , 0 , sizeof( serv_addr )) ;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons( 22 );
    //serv_addr.sin_addr.s_addr = 
    struct hostent * remote_host_ipaddrs = ::gethostbyname(this->host_name.c_str());
    char * ent_pos_c = 0 ;
    int counter = 0 ;
    printf("remote_host ip: %s \n",remote_host_ipaddrs->h_name);

    ent_pos_c = remote_host_ipaddrs->h_addr_list[0];
    while(ent_pos_c!= NULL )
    {
        printf("host addr: %s -> %s  \n", ent_pos_c , inet_ntop(AF_INET,ent_pos_c,host_ipadd,sizeof(host_ipadd) ) );
        ent_pos_c =  remote_host_ipaddrs->h_addr_list[++counter];
    }
    
    ret = inet_pton(AF_INET , host_ipadd ,&serv_addr.sin_addr.s_addr);
    printf(" inet_pton ret: %d \n" , ret );
    
    this->ssh2_sock = socket(AF_INET,SOCK_STREAM,0);
    ret = ::connect( this->ssh2_sock,(struct sockaddr*)&serv_addr,sizeof(serv_addr));
    assert( ret == 0 );
    
    //create session
    ssh2_sess = libssh2_session_init();
    
    ret = libssh2_session_startup((LIBSSH2_SESSION*)ssh2_sess,this->ssh2_sock);
    assert( ret == 0 );

    ///////////
    //auth
    char * auth_list = libssh2_userauth_list((LIBSSH2_SESSION*)ssh2_sess,this->user_name.c_str(),strlen(this->user_name.c_str()) );
    printf("user auth list : %s \n" , auth_list ) ;
    
    strncpy(ssh2_password,this->password.c_str() , sizeof(ssh2_password));
    
    ret = libssh2_userauth_keyboard_interactive((LIBSSH2_SESSION*)ssh2_sess,this->user_name.c_str(),&kbd_callback) ;
    qDebug()<<"keyboard interactive :"<<ret ;
    if( ret == -1 )
    {
        ret = libssh2_userauth_password((LIBSSH2_SESSION*)ssh2_sess,this->user_name.c_str(),ssh2_password);
        qDebug()<<"auth_password :"<<ret ;
    }
    
    ret = libssh2_userauth_authenticated((LIBSSH2_SESSION*)ssh2_sess);
    if( ret == 0 )
    {
        this->connect_status = 1 ;
        qDebug()<<" user auth faild";
        return ;
    }
    ssh2_sftp = libssh2_sftp_init((LIBSSH2_SESSION*)ssh2_sess );
    assert( ssh2_sftp != NULL );

    
    ret = libssh2_sftp_realpath((LIBSSH2_SFTP*)ssh2_sftp,".",home_path,PATH_MAX);
    if(ret != 0 )
    {
        qDebug()<<" realpath : "<< ret  
                << " err code : " << libssh2_sftp_last_error((LIBSSH2_SFTP*)ssh2_sftp) 
                << home_path ;
        
        assert( ret >= 0 );
    }
    this->user_home_path = std::string( home_path );
    
    this->connect_status = 0 ;
}

void RemoteHostConnectThread::slot_finished()
{
    emit this->connect_finished(this->connect_status,this->ssh2_sess,this->ssh2_sock,this->ssh2_sftp );
}

void RemoteHostConnectThread::do_init()
{
    qDebug() <<__FUNCTION__<<": "<<__LINE__<<":"<< __FILE__;

}

void RemoteHostConnectThread::diconnect_ssh_connection()
{
    qDebug() <<__FUNCTION__<<": "<<__LINE__<<":"<< __FILE__;
    
//     int ret = 0 ;
//     if( this->child_pid > 0 )
//     {
//         ret = kill(this->child_pid,SIGHUP);
//         qDebug() << "attemp to kill child process :" << this->child_pid <<" ret:"<<ret ;
//         ret = kill(this->child_pid,SIGTERM);
//         qDebug() << "attemp to kill child process :" << this->child_pid <<" ret:"<<ret ;
//         ret = kill(this->child_pid,SIGABRT);
//         qDebug() << "attemp to kill child process :" << this->child_pid <<" ret:"<<ret ;
//         waitpid(this->child_pid, NULL, 0);
//     }
}

std::string RemoteHostConnectThread::get_user_home_path () 
{
    return this->user_home_path ;
}

