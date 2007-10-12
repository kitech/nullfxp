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

#include <sys/types.h>
#include <errno.h>
#include <signal.h>

#include <sys/types.h>

#ifdef WIN32
#include <winsock2.h>
#else
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <wait.h>
#endif

#include <assert.h>

#include <QtCore>

#include "progressdialog.h"

#include "remotehostconnectthread.h"

#include "libssh2.h"
#include "libssh2_sftp.h"

//static char ssh2_user_name[60];
static QMutex ssh2_kbd_cb_mutex ;

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
    //this->start();
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
    char host_ipaddr[60] = {0};
    
    
#ifdef WIN32
		#define WINSOCK_VERSION 2.0
    WSADATA wsadata;

    WSAStartup(WINSOCK_VERSION, &wsadata);
#endif

    //create socket 
    struct sockaddr_in serv_addr ;
    memset( & serv_addr , 0 , sizeof( serv_addr )) ;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons( 22 );
    //serv_addr.sin_addr.s_addr = 
    
    emit connect_state_changed(tr("Resoving %1 ...").arg(this->host_name.c_str()));
    struct hostent * remote_host_ipaddrs = ::gethostbyname(this->host_name.c_str());
    char * ent_pos_c = 0 ;
    int counter = 0 ;
    if( remote_host_ipaddrs == 0 )
    {
    	#ifdef WIN32
    		emit connect_state_changed( tr( "Resoving host faild :  ") ) ;
    	#else
        emit connect_state_changed( tr( "Resoving host faild : (%1),%2 ").arg(h_errno).arg(hstrerror(h_errno)) ) ;
      #endif
        this->connect_status = 1 ;
        //assert( ret == 0 );
        return ;
    }
    printf("remote_host name is : %s \n",remote_host_ipaddrs->h_name);

    ent_pos_c = remote_host_ipaddrs->h_addr_list[0];
    while(ent_pos_c!= NULL )
    {
    	#ifdef WIN32
    		struct in_addr tmp_in_addr ;
    		memset(&tmp_in_addr,0,sizeof(struct in_addr));
    		memcpy(&tmp_in_addr,ent_pos_c,sizeof(struct in_addr));
      	printf("host addr: %s -> %s \n", ent_pos_c , inet_ntoa( tmp_in_addr ) );
      	strcpy( host_ipaddr,inet_ntoa( tmp_in_addr ) );
      #else
        printf("host addr: %s -> %s  \n", ent_pos_c , inet_ntop(AF_INET,ent_pos_c,host_ipaddr , sizeof(host_ipaddr) ) );
      #endif  
        ent_pos_c =  remote_host_ipaddrs->h_addr_list[++counter];
        emit connect_state_changed( tr("Remote host IP: %1").arg(host_ipaddr) );
    }
    
    #ifdef WIN32
    serv_addr.sin_addr.s_addr = (unsigned long)inet_addr ( host_ipaddr ) ;
    #else
    ret = inet_pton(AF_INET , host_ipaddr ,&serv_addr.sin_addr.s_addr);
    printf(" inet_pton ret: %d \n" , ret );
    #endif   
    
    emit connect_state_changed( tr("Connecting to %1 ( %2 ) ").arg(this->host_name.c_str()).arg(host_ipaddr) );
    this->ssh2_sock = socket(AF_INET,SOCK_STREAM,0);
    ret = ::connect( this->ssh2_sock,(struct sockaddr*)&serv_addr,sizeof(serv_addr));
    if( ret != 0 )
    {
        emit connect_state_changed( tr( "Connect faild : (%1),%2 ").arg(errno).arg(strerror(errno)) ) ;
        this->connect_status = 1 ;
        //assert( ret == 0 );
        return ;
    }
    
    //create session
    ssh2_sess = libssh2_session_init();
    //libssh2_trace((LIBSSH2_SESSION*)ssh2_sess , 64 );    
    ret = libssh2_session_startup((LIBSSH2_SESSION*)ssh2_sess,this->ssh2_sock);
    assert( ret == 0 );
    emit connect_state_changed( tr( "SSH session started ..."));
    
    ///////////
    //auth
    char * auth_list = libssh2_userauth_list((LIBSSH2_SESSION*)ssh2_sess,this->user_name.c_str(),strlen(this->user_name.c_str()) );
    printf("user auth list : %s \n" , auth_list ) ;
    
    emit connect_state_changed( tr( "User authing (Keyboard Interactive)...") );
    
    ssh2_kbd_cb_mutex.lock();
    strncpy(ssh2_password,this->password.c_str() , sizeof(ssh2_password));
    ret = libssh2_userauth_keyboard_interactive((LIBSSH2_SESSION*)ssh2_sess,this->user_name.c_str(),&kbd_callback) ;
    memset( ssh2_password,0,sizeof(ssh2_password));
    ssh2_kbd_cb_mutex.unlock();
    
    qDebug()<<"keyboard interactive :"<<ret ;
    if( ret == -1 )
    {
        emit connect_state_changed( tr( "User auth faild (Keyboard Interactive). Trying (Password ) ...") );
        ret = libssh2_userauth_password((LIBSSH2_SESSION*)ssh2_sess,this->user_name.c_str(),this->password.c_str());
        qDebug()<<"auth_password :"<<ret ;
    }
    
    ret = libssh2_userauth_authenticated((LIBSSH2_SESSION*)ssh2_sess);
    if( ret == 0 )
    {
        this->connect_status = 1 ;
        qDebug()<<" user auth faild";
        emit connect_state_changed( tr( "User faild (Keyboard Interactive)(Password ).") );
        return ;
    }
    
    ssh2_sftp = libssh2_sftp_init((LIBSSH2_SESSION*)ssh2_sess );
    assert( ssh2_sftp != NULL );

    emit connect_state_changed(tr("User auth successfully"));
    ret = libssh2_sftp_realpath((LIBSSH2_SFTP*)ssh2_sftp,".",home_path,PATH_MAX);
    if(ret != 0 )
    {
        qDebug()<<" realpath : "<< ret  
                << " err code : " << libssh2_sftp_last_error((LIBSSH2_SFTP*)ssh2_sftp) 
                << home_path ;
        
        assert( ret >= 0 );
    }
    this->user_home_path = std::string( home_path );
    libssh2_sftp_shutdown( (LIBSSH2_SFTP*) this->ssh2_sftp );
    this->connect_status = 0 ;
}

void RemoteHostConnectThread::slot_finished()
{
    emit this->connect_finished(this->connect_status,this->ssh2_sess,this->ssh2_sock/*,this->ssh2_sftp*/ );
}

void RemoteHostConnectThread::do_init()
{
    qDebug() <<__FUNCTION__<<": "<<__LINE__<<":"<< __FILE__;
}

void RemoteHostConnectThread::diconnect_ssh_connection()
{
    qDebug() <<__FUNCTION__<<": "<<__LINE__<<":"<< __FILE__;
}

std::string RemoteHostConnectThread::get_user_home_path () 
{
    return this->user_home_path ;
}

QString RemoteHostConnectThread::get_host_name()
{
    return QString(this->host_name.c_str());
}

QString RemoteHostConnectThread::get_user_name()
{
    return this->user_name.c_str() ;
}
QString RemoteHostConnectThread::get_password ()
{
    return this->password.c_str()  ;
}

void * RemoteHostConnectThread::get_ssh2_sess () 
{
    return this->ssh2_sess ;
}
int RemoteHostConnectThread::get_ssh2_sock () 
{
    return this->ssh2_sock ;
}

