// remotehostconnectthread.cpp --- 
// 
// Filename: remotehostconnectthread.cpp
// Description: 
// Author: 刘光照<liuguangzhao@users.sf.net>
// Maintainer: 
// Copyright (C) 2007-2008 liuguangzhao <liuguangzhao@users.sf.net>
// http://www.qtchina.net
// http://nullget.sourceforge.net
// Created: 六  6月 14 22:29:50 2008 (CST)
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
//#include <wait.h>
#include <netinet/in.h>
#endif

#include <fcntl.h>
#include <assert.h>

#include <QtCore>

#include "progressdialog.h"

#include "remotehostconnectthread.h"

#include "libssh2.h"
#include "libssh2_sftp.h"

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


RemoteHostConnectThread::RemoteHostConnectThread(QString user_name, QString password, 
                                                 QString host_name, short port, QString pubkey,  
                                                 QObject* parent)
    : QThread(parent)
{
    this->user_name = user_name;
    this->password = password;
    //this->decoded_password = QUrl::fromPercentEncoding(this->password.toAscii());
    this->host_name = host_name;
    this->port = port;
    this->pubkey_path = pubkey;
    if(this->pubkey_path.length() == 0) {
        this->pubkey_path = QString::null;
    }
    if(!QFile::exists(this->pubkey_path)) {
        qDebug()<<"Warning: pubkey setted, but file not exist";
    }
    this->connect_status = CONN_OK;
    this->user_canceled = false;
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
    serv_addr.sin_port = htons(this->port);
    //serv_addr.sin_addr.s_addr = 
    
    emit connect_state_changed(tr("Resoving %1 ...").arg(this->host_name));
    struct hostent * remote_host_ipaddrs = ::gethostbyname(this->host_name.toAscii().data());
    char * ent_pos_c = 0 ;
    int counter = 0 ;
    if( remote_host_ipaddrs == 0 )
    {
#ifdef WIN32
        emit connect_state_changed( tr( "Resoving host faild : (%1),%2  ").arg(errno).arg(strerror(errno)) ) ;
#else
        emit connect_state_changed( tr( "Resoving host faild : (%1),%2 ").arg(h_errno).arg(hstrerror(h_errno)) ) ;
#endif
        this->connect_status = CONN_RESOLVE_ERROR ;
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
    if( this->user_canceled == true ){
        this->connect_status = CONN_CANCEL ;
        return;
    }   

    emit connect_state_changed( tr("Connecting to %1 ( %2:%3 ) ").arg(this->host_name).arg(host_ipaddr).arg(this->port) );
    this->ssh2_sock = socket(AF_INET,SOCK_STREAM,0);
    assert(this->ssh2_sock > 0);
    //设置连接超时
    unsigned long sock_flag = 1;
#ifdef WIN32
    ioctlsocket(this->ssh2_sock, FIONBIO, &sock_flag);
#else
    sock_flag = fcntl(this->ssh2_sock, F_GETFL);
    fcntl(this->ssh2_sock, F_SETFL, sock_flag | O_NONBLOCK);
#endif
    ret = ::connect( this->ssh2_sock,(struct sockaddr*)&serv_addr,sizeof(serv_addr));

    struct timeval timeo = {10, 0};//连接超时10秒
    fd_set set;
    FD_ZERO(&set);
    FD_SET(this->ssh2_sock, &set);
#ifdef WIN32
    ret = select(this->ssh2_sock + 1, NULL, &set, NULL, &timeo);
#else
    ret = select(this->ssh2_sock + 1, NULL, &set, NULL, &timeo);
#endif
    if( ret == -1){
        assert( 1==2);
    }else if( ret == 0 ) {
        QString emsg = QString(tr( "Connect faild : (%1),%2 ").arg(errno).arg(strerror(errno)));
        emit connect_state_changed( emsg ) ;
        this->connect_status = CONN_REFUSE ;
        qDebug()<<emsg;
        //assert( ret == 0 );
        return ;
    }
#ifdef WIN32
    sock_flag = 0;
    ioctlsocket(this->ssh2_sock, FIONBIO, &sock_flag);
#else
    fcntl(this->ssh2_sock, F_SETFL, sock_flag);
#endif
    if( this->user_canceled == true ){
        this->connect_status = 2 ;
#ifdef WIN32
        ::closesocket(this->ssh2_sock);
#else
        ::close(this->ssh2_sock);
#endif
        return;
    }   

    //create session
    ssh2_sess = libssh2_session_init();
    //libssh2_trace((LIBSSH2_SESSION*)ssh2_sess , 64 );    
    ret = libssh2_session_startup((LIBSSH2_SESSION*)ssh2_sess,this->ssh2_sock);
    printf("Received Banner: %s\n",libssh2_session_get_remote_version((LIBSSH2_SESSION*)ssh2_sess));
    if(ret != 0) {
        this->connect_status = CONN_SESS_ERROR;
#ifdef WIN32
        ::closesocket(this->ssh2_sock);
#else
        ::close(this->ssh2_sock);
#endif
        assert( ret == 0 );
        return ;
    }
    emit connect_state_changed( tr( "SSH session started ..."));
    
    ///////////
    //auth
    char * auth_list = libssh2_userauth_list((LIBSSH2_SESSION*)ssh2_sess,this->user_name.toAscii().data(),strlen(this->user_name.toAscii().data()) );
    printf("user auth list : %s \n" , auth_list ) ;

    ret = libssh2_userauth_hostbased_fromfile((LIBSSH2_SESSION*)ssh2_sess, 
                                              this->user_name.toAscii().data(),
                                              this->pubkey_path.toAscii().data(),
                                              this->pubkey_path.left(this->pubkey_path.length()-4).toAscii().data(),
                                              //this->decoded_password.toAscii().data(),
                                              QUrl::fromPercentEncoding(this->password.toAscii()).toAscii().data(),
                                              this->host_name.toAscii().data());
    //qDebug()<<this->user_name<<this->pubkey_path<<this->pubkey_path.left(this->pubkey_path.length()-4)
    //      <<this->decoded_password<<this->host_name;;

    if(ret == -1){
        char * emsg = 0;
        int  emsg_len = 0;
        libssh2_session_last_error((LIBSSH2_SESSION*)ssh2_sess, &emsg, &emsg_len, 1);
        qDebug()<<"Host based public key auth error: "<<emsg;
        emit connect_state_changed( QString("%1%2").arg(tr("Host based public key auth error: ")).arg(emsg));
        if(emsg != 0) free(emsg);        
    }else{
        qDebug()<<"Host based public key auth successful";
        emit connect_state_changed( QString("%1").arg(tr("Host based public key auth successful")));
    }

    if(ret == -1) {
        if(this->pubkey_path != QString::null && this->pubkey_path.length() != 0) {
            //publickey auth
            emit connect_state_changed(QString(tr("User authing(PublicKey: %1)...")).arg(this->pubkey_path));
            if( this->user_canceled == true ){
                this->connect_status = 2 ;
                libssh2_session_disconnect((LIBSSH2_SESSION*)ssh2_sess,"");
                libssh2_session_free((LIBSSH2_SESSION*)ssh2_sess);
#ifdef WIN32
                ::closesocket(this->ssh2_sock);
#else
                ::close(this->ssh2_sock);
#endif
                return;
            }
            ret = libssh2_userauth_publickey_fromfile((LIBSSH2_SESSION*)ssh2_sess, 
                                                      this->user_name.toAscii().data(),
                                                      this->pubkey_path.toAscii().data(),
                                                      this->pubkey_path.left(this->pubkey_path.length()-4).toAscii().data(),
                                                      //this->decoded_password.toAscii().data());
                                                      QUrl::fromPercentEncoding(this->password.toAscii()).toAscii().data());                                                     


            //qDebug()<<this->user_name<<this->pubkey_path<<this->pubkey_path.left(this->pubkey_path.length()-4)
            //      <<this->decoded_password;
        }else{
            ret = -1;
        }
    }
    if(ret == 0) {
        qDebug()<<"PublicKey auth successfully";
    }
    if(ret == -1) {
        //password auth
        if(this->pubkey_path == QString::null || this->pubkey_path.length() == 0) {
            emit connect_state_changed( tr( "User authing (Password)...") );    
        }else{
            char * emsg = 0;
            int  emsg_len = 0;
            libssh2_session_last_error((LIBSSH2_SESSION*)ssh2_sess, &emsg, &emsg_len, 1);
            qDebug()<<"PublicKey auth error: "<<emsg;
            if(emsg != 0) free(emsg);
            
            emit connect_state_changed( tr( "User auth faild(PublicKey). Trying (Password)...") );    
        }
        if( this->user_canceled == true ){
            this->connect_status = 2 ;
            libssh2_session_disconnect((LIBSSH2_SESSION*)ssh2_sess,"");
            libssh2_session_free((LIBSSH2_SESSION*)ssh2_sess);
#ifdef WIN32
            ::closesocket(this->ssh2_sock);
#else
            ::close(this->ssh2_sock);
#endif
            return;
        }   

        ret = libssh2_userauth_password((LIBSSH2_SESSION*)ssh2_sess,this->user_name.toAscii().data(),
                                        //this->decoded_password.toAscii().data());
                                        QUrl::fromPercentEncoding(this->password.toAscii()).toAscii().data());
        qDebug()<<"Keyboard Interactive :"<<ret ;

        if( ret == -1 ) {
            emit connect_state_changed( tr( "User auth faild (Password). Trying (Keyboard Interactive) ...") );
            ssh2_kbd_cb_mutex.lock();
            strncpy(ssh2_password, 
                    //this->decoded_password.toAscii().data(),
                    QUrl::fromPercentEncoding(this->password.toAscii()).toAscii().data(),
                    sizeof(ssh2_password));
            ret = libssh2_userauth_keyboard_interactive((LIBSSH2_SESSION*)ssh2_sess,this->user_name.toAscii().data(),&kbd_callback) ;
            memset( ssh2_password,0,sizeof(ssh2_password));
            ssh2_kbd_cb_mutex.unlock();
            qDebug()<<"Keyboard interactive :"<<ret ;
        }
        if( this->user_canceled == true ){
            this->connect_status = 2 ;
            libssh2_session_disconnect((LIBSSH2_SESSION*)ssh2_sess,"");
            libssh2_session_free((LIBSSH2_SESSION*)ssh2_sess);
#ifdef WIN32
            ::closesocket(this->ssh2_sock);
#else
            ::close(this->ssh2_sock);
#endif
            return;
        }   
    }
    ret = libssh2_userauth_authenticated((LIBSSH2_SESSION*)ssh2_sess);
    if( ret == 0 ) {
        this->connect_status = CONN_AUTH_ERROR ;
        qDebug()<<"User auth faild";
        emit connect_state_changed( tr( "User faild (Keyboard Interactive)(Password ).") );
        return ;
    }
    if( this->user_canceled == true ){
        this->connect_status = CONN_CANCEL ;
        libssh2_session_disconnect((LIBSSH2_SESSION*)ssh2_sess,"");
        libssh2_session_free((LIBSSH2_SESSION*)ssh2_sess);
#ifdef WIN32
        ::closesocket(this->ssh2_sock);
#else
        ::close(this->ssh2_sock);
#endif
        return;
    }   
    
    emit connect_state_changed(tr("User auth successfully"));
    
    ssh2_sftp = libssh2_sftp_init((LIBSSH2_SESSION*)ssh2_sess );
    assert( ssh2_sftp != NULL );
    char **server_info, **pptr;
    server_info = pptr = libssh2_session_get_remote_info((LIBSSH2_SESSION*)ssh2_sess);
    printf("Received SFTP Version: %d %s\n",libssh2_sftp_get_version((LIBSSH2_SFTP*)ssh2_sftp), server_info[0]);	
    while(*pptr != NULL){
        free(*pptr); pptr++;
    }
    free(server_info);
    
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
    
    if( this->user_canceled == true ){
        this->connect_status = CONN_CANCEL ;
        libssh2_session_disconnect((LIBSSH2_SESSION*)ssh2_sess,"");
        libssh2_session_free((LIBSSH2_SESSION*)ssh2_sess);
#ifdef WIN32
        ::closesocket(this->ssh2_sock);
#else
        ::close(this->ssh2_sock);
#endif
        return;
    }   

    //getevn
    this->get_server_env_vars("env");
    this->get_server_env_vars("uname -a");
    //qDebug()<<"MD5 Hostkey hash:"<<libssh2_hostkey_hash((LIBSSH2_SESSION*)ssh2_sess,LIBSSH2_HOSTKEY_HASH_MD5 )
    //<<"\nSHA1 Hostkey hash:"<<libssh2_hostkey_hash((LIBSSH2_SESSION*)ssh2_sess, LIBSSH2_HOSTKEY_HASH_SHA1);
}

QString RemoteHostConnectThread::get_server_env_vars(char *cmd)
{
    LIBSSH2_CHANNEL * ssh2_channel = 0;
    int rv = -1;
    char buff[1024] ;
    QString evn_output;
    QString uname_output;

    ssh2_channel = libssh2_channel_open_session((LIBSSH2_SESSION*)this->ssh2_sess);
    //libssh2_channel_set_blocking(ssh2_channel, 1);
    rv = libssh2_channel_exec(ssh2_channel, cmd);
    qDebug()<<"SSH2 exec: "<<rv;
  
    memset(buff, 0, sizeof(buff));
    while( (rv = libssh2_channel_read(ssh2_channel, buff, 1000)) > 0){
        qDebug()<<"Channel read: "<<rv<<" -->"<<buff;
        memset(buff, 0, sizeof(buff));
    }

    libssh2_channel_close(ssh2_channel);
    libssh2_channel_free(ssh2_channel);

    return QString();
}

void RemoteHostConnectThread::slot_finished()
{
    emit this->connect_finished(this->connect_status,this->ssh2_sess,this->ssh2_sock);
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
    return this->host_name;
}

QString RemoteHostConnectThread::get_user_name()
{
    return this->user_name ;
}
QString RemoteHostConnectThread::get_password ()
{
    return this->password  ;
}
short   RemoteHostConnectThread::get_port()
{
    return this->port;
}
QString RemoteHostConnectThread::get_pubkey()
{
    return this->pubkey_path;
}
void * RemoteHostConnectThread::get_ssh2_sess () 
{
    return this->ssh2_sess ;
}
int RemoteHostConnectThread::get_ssh2_sock () 
{
    return this->ssh2_sock ;
}

int RemoteHostConnectThread::get_connect_status()
{
    return this->connect_status;
}

void RemoteHostConnectThread::set_user_canceled()
{
    this->user_canceled = true;
}

QString RemoteHostConnectThread::get_status_desc(int status)
{
    static char * status_desc[] = {
        "CONN_OK",
        "CONN_REFUSE",
        "CONN_CANCEL",
        "CONN_OTHER",
        "CONN_RESOLVE_ERROR",
        "CONN_SESS_ERROR",
        "CONN_AUTH_ERROR",
        "CONN_SFTP_ERROR",
        "CONN_EXEC_ERROR"
    };
    
    if(status > sizeof(status_desc)/sizeof(char*) ) {
        return "Unknown status";
    }else{
        return status_desc[status];
    }
}

//TODO 在网络断线时这run中的代码可能导致程序崩溃
/*

remote_host name is : 60.2.236.22
host addr: <� -> 60.2.236.22
inet_pton ret: 1

*/



