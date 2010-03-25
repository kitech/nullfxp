// sshconnection.cpp --- 
// 
// Author: liuguangzhao
// Copyright (C) 2007-2010 liuguangzhao@users.sf.net
// URL: http://www.qtchina.net http://nullget.sourceforge.net
// Created: 2009-09-06 10:35:47 +0800
// Version: $Id$
// 

#include <cstdlib>
#include <cstdio>
#include <cassert>
#include <cerrno>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/types.h>

#ifndef _MSC_VER
#include <fcntl.h>
#endif

#ifdef WIN32
#include <winsock2.h>
#else
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#endif

#include "libssh2.h"
#include "libssh2_sftp.h"
#include "info.h"

#include "utils.h"
#include "globaloption.h"

#include "sshconnection.h"

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


SSHConnection::SSHConnection(QObject *parent)
    : Connection(parent)
{
}
SSHConnection::~SSHConnection()
{
    q_debug()<<"";
    if (this->qsock != NULL) {
        delete this->qsock;
        this->qsock = NULL;
    }
    if (this->qdsock != NULL) {
        delete this->qdsock;
        this->qdsock = NULL;
    }
}

int SSHConnection::connect()
{
    qDebug()<<__FUNCTION__<<": "<<__LINE__<<":"<<__FILE__;
    int ret = 0;
    
    ret = this->initSocket();
    if (ret != 0) {
        q_debug()<<"socket init error"<<ret<<this->get_status_desc(ret);;
        return ret;
    }

    //create session
    ret = this->initSSHSession();
    if (ret != 0) {
        q_debug()<<"sesion error"<<ret<<this->get_status_desc(ret);;
        return ret;
    }
    
    ///////////
    //auth
    ret = this->sshAuth();
    if (ret != 0) {
        q_debug()<<"auth error"<<ret<<this->get_status_desc(ret);;
        return ret;
    }        
    
    // home path
    ret = this->sshHomePath();
    if (ret != 0) {
        q_debug()<<"home path error"<<ret<<this->get_status_desc(ret);;
        return ret;
    }

    //getevn
    QString envs = this->get_server_env_vars("env");
    this->codec = this->codecForEnv(envs);
    this->get_server_env_vars("uname -a");
    //qDebug()<<"MD5 Hostkey hash:"<<libssh2_hostkey_hash(this->sess,LIBSSH2_HOSTKEY_HASH_MD5 )
    //<<"\nSHA1 Hostkey hash:"<<libssh2_hostkey_hash(this->sess, LIBSSH2_HOSTKEY_HASH_SHA1);
    return 0;
}
int SSHConnection::disconnect()
{
    return 0;
}
int SSHConnection::reconnect()
{
    return 0;
}
bool SSHConnection::isConnected()
{
    return Connection::isConnected();
}
bool SSHConnection::isRealConnected()
{
    return Connection::isRealConnected();
}

int SSHConnection::alivePing()
{
    return 0;
}
QTextCodec *SSHConnection::codecForEnv(QString env)
{
    QTextCodec *ecodec = NULL;
    QStringList sl = env.split("\n");
    QString kvline;
    for (int i = 0 ; i < sl.count(); i++) {
        kvline = sl.at(i);
        if (kvline.startsWith("LANG=")
            || kvline.startsWith("LC_CTYPE=")
            || kvline.startsWith("LC_ALL=")) {
            if (kvline.split(".").count() == 2) {
                ecodec = QTextCodec::codecForName(kvline.split(".").at(1).toAscii());
                if (ecodec != NULL) {
                    qDebug()<<"Got env encoding:"<<kvline;
                    break;
                }
            }
        }
    }
    if (ecodec == NULL) {
        q_debug()<<"can not got encoding from env";
    }
    return ecodec;
}
/////////////// private
int SSHConnection::initSocket()
{
    int ret = -1;

#ifdef WIN32
#define WINSOCK_VERSION 2.0
    WSADATA wsadata;
    WSAStartup(WINSOCK_VERSION, &wsadata);
#endif

    //create socket 
    struct sockaddr_in serv_addr ;
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(this->port);

    emit connect_state_changed(tr("Resoving %1 ...").arg(this->hostName));
    // resolve host ip using qt class
    QHostAddress dest_addr = QHostAddress(this->hostName); // for literal IP
    if (dest_addr.isNull()) {
        QHostInfo hi = QHostInfo::fromName(this->hostName);
        if (hi.error() != QHostInfo::NoError) {
            // this->connect_status = CONN_RESOLVE_ERROR;
            emit connect_state_changed(tr("Resoving host faild: (%1),%2").arg(hi.error())
                                       .arg(hi.errorString()));
            return CONN_RESOLVE_ERROR;;
        } else {
            dest_addr = hi.addresses().at(0);
        }
    }

#ifdef WIN32
    serv_addr.sin_addr.s_addr = (unsigned long)inet_addr(dest_addr.toString().toAscii().data());
#else
    ret = inet_pton(AF_INET, dest_addr.toString().toAscii().data(), &serv_addr.sin_addr.s_addr);
#endif

    if (this->user_canceled == true) {
        // this->connect_status = CONN_CANCEL ;
        return CONN_CANCEL;
    }   

    emit connect_state_changed(tr("Connecting to %1 ( %2:%3 )").arg(this->hostName)
                               .arg(dest_addr.toString()).arg(this->port));
    this->sock = socket(AF_INET, SOCK_STREAM, 0);
    assert(this->sock > 0);

    //设置连接超时
    unsigned long sock_flag = 1;
#ifdef WIN32
    ioctlsocket(this->sock, FIONBIO, &sock_flag);
#else
    sock_flag = fcntl(this->sock, F_GETFL);
    fcntl(this->sock, F_SETFL, sock_flag | O_NONBLOCK);
#endif
    ret = ::connect(this->sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr));
    QTextCodec *codec = gOpt->locale_codec;

    struct timeval timeo = {10, 0}; //连接超时10秒
    fd_set set, rdset, exset;
    FD_ZERO(&set);
    FD_SET(this->sock, &set);
    rdset = set;
    exset = set;
    ret = select(this->sock + 1, &rdset, &set, &exset, &timeo);
// #ifdef WIN32
//     ret = select(this->sock + 1, &rdset, &set, &exset, &timeo);
// #else
//     ret = select(this->sock + 1, &rdset, &set, &exset, &timeo);
// #endif
    if (ret == -1) {
        assert(1 == 2);
    } else if (ret == 0) {
        QString emsg = QString(tr("Connect faild : (%1),%2 ").arg(errno).arg(strerror(errno)));
        emit connect_state_changed(emsg) ;
        // this->connect_status = CONN_REFUSE;
        qDebug()<<emsg;
        //assert( ret == 0 );
        return CONN_REFUSE;
    } else {
#ifndef WIN32
        if (!FD_ISSET(this->sock, &rdset) && !FD_ISSET(this->sock, &set)) {
            qDebug()<<this->sock<<ret<<errno<<codec->toUnicode(QByteArray(strerror(errno)));
        }
        int myerrno = 888;
        socklen_t mylen = 889;
        if (getsockopt(this->sock, SOL_SOCKET, SO_ERROR, &myerrno, &mylen) < 0) {
            qDebug()<<"getsockopt error:";            
        }
        if (myerrno != 0) {
            qDebug()<<"Connect faild: "<<codec->toUnicode(QByteArray(strerror(myerrno)));
            emit connect_state_changed(QString("%1%2").arg(tr("Connect error: "))
                                       .arg(codec->toUnicode(QByteArray(strerror(myerrno)))));
            // this->connect_status = CONN_REFUSE ;            
            return CONN_REFUSE;
        }
#endif
    }    

#ifdef WIN32
    sock_flag = 0;
    ret = ioctlsocket(this->sock, FIONBIO, &sock_flag);
    if (ret == SOCKET_ERROR) {
    		qDebug()<<"win connect error";
    		emit connect_state_changed(QString("%1%2").arg(tr("Connect error: "))
                                       .arg(codec->toUnicode(QByteArray(strerror(ret)))));
            // this->connect_status = CONN_REFUSE ;
            return CONN_REFUSE;
    }
#else
    sock_flag = 0;
    fcntl(this->sock, F_SETFL, sock_flag);
#endif

    if (this->user_canceled == true) {
        this->piClose(this->sock);
        return CONN_CANCEL;
    }   

    return 0;
}

int SSHConnection::initSSHSession()
{
    int ret = -1;

    this->sess = libssh2_session_init();
    libssh2_trace(this->sess, 64);
    ret = libssh2_session_startup(this->sess, this->sock);
    if (ret != 0) {
        {
            char *emsg = 0;
            int  emsg_len = 0;
            libssh2_session_last_error(this->sess, &emsg, &emsg_len, 1);
            qDebug()<<"Start ssh session error: "<<libssh2_session_last_errno(this->sess)
                    <<emsg;
            emit connect_state_changed(QString("%1%2").arg(tr("Start ssh session error: ")).arg(emsg));
            if (emsg != 0) free(emsg);
        }        
        if (ret == LIBSSH2_ERROR_KEX_FAILURE) {
            // maybe the server ssh protocol version is 1.x
            // this->connect_status = CONN_PROTOCOL_VERSION_NOT_MATCH_ERROR;
            this->piClose(this->sock);
            return CONN_PROTOCOL_VERSION_NOT_MATCH_ERROR;
        } else {
            // this->connect_status = CONN_SESS_ERROR;
            this->piClose(this->sock);
            return CONN_SESS_ERROR;
        }
        //assert( ret == 0 );
        return CONN_OK;
    }
    printf("Received Banner: %s\n", libssh2_session_get_remote_version(this->sess));
    emit connect_state_changed(tr("SSH session started ..."));

    return 0;
}
int SSHConnection::sshAuth()
{
    int ret = -1;
    char *auth_list = libssh2_userauth_list(this->sess,
                                            this->userName.toAscii().data(), 
                                            strlen(this->userName.toAscii().data()));
    printf("user auth list : %s \n" , auth_list);

    ret = libssh2_userauth_hostbased_fromfile(this->sess, 
                                              this->userName.toAscii().data(),
                                              this->pubkey.toAscii().data(),
                                              this->pubkey.left(this->pubkey.length()-4).toAscii().data(),
                                              QUrl::fromPercentEncoding(this->password.toAscii()).toAscii().data(),
                                              this->hostName.toAscii().data());
    //qDebug()<<this->user_name<<this->pubkey_path<<this->pubkey_path.left(this->pubkey_path.length()-4)
    //      <<this->decoded_password<<this->host_name;;

    if (ret == -1) {
        char *emsg = 0;
        int  emsg_len = 0;
        libssh2_session_last_error(this->sess, &emsg, &emsg_len, 1);
        qDebug()<<"Host based public key auth error: "<<emsg;
        emit connect_state_changed( QString("%1%2").arg(tr("Host based public key auth error: ")).arg(emsg));
        if (emsg != 0) free(emsg);        
    } else {
        qDebug()<<"Host based public key auth successful";
        emit connect_state_changed(QString("%1").arg(tr("Host based public key auth successful")));
    }

    if (ret == -1) {
        if (this->pubkey != QString::null && this->pubkey.length() != 0) {
            //publickey auth
            emit connect_state_changed(QString(tr("User authing(PublicKey: %1)...")).arg(this->pubkey));
            if (this->user_canceled == true) {
                // this->connect_status = 2 ;
                libssh2_session_disconnect(this->sess, "");
                libssh2_session_free(this->sess);
                this->piClose(this->sock);
                return CONN_CANCEL;
            }

            ret = libssh2_userauth_publickey_fromfile(this->sess, 
                                                      this->userName.toAscii().data(),
                                                      this->pubkey.toAscii().data(),
                                                      this->pubkey.left(this->pubkey.length()-4).toAscii().data(),
                                                      QUrl::fromPercentEncoding(this->password.toAscii()).toAscii().data());            //qDebug()<<this->user_name<<this->pubkey_path<<this->pubkey_path.left(this->pubkey_path.length()-4)
            //      <<this->decoded_password;
        } else {
            ret = -1;
        }
    }
    if (ret == 0) {
        qDebug()<<"PublicKey auth successfully";
    }
    if (ret == -1) {
        //password auth
        if (this->pubkey == QString::null || this->pubkey.length() == 0) {
            emit connect_state_changed(tr("User authing (Password)..."));
        } else {
            char *emsg = 0;
            int  emsg_len = 0;
            libssh2_session_last_error(this->sess, &emsg, &emsg_len, 1);
            qDebug()<<"PublicKey auth error: "<<emsg;
            if (emsg != 0) free(emsg);
            
            emit connect_state_changed(tr( "User auth faild(PublicKey). Trying (Password)..."));
        }
        if (this->user_canceled == true) {
            // this->connect_status = 2 ;
            libssh2_session_disconnect(this->sess, "");
            libssh2_session_free(this->sess);
            this->piClose(this->sock);
            return CONN_CANCEL;
        }   

        ret = libssh2_userauth_password(this->sess, this->userName.toAscii().data(),
                                        QUrl::fromPercentEncoding(this->password.toAscii()).toAscii().data());
        qDebug()<<"Keyboard Interactive :"<<ret;

        if (ret == -1) {
            emit connect_state_changed(tr("User auth faild (Password). Trying (Keyboard Interactive) ..."));
            ssh2_kbd_cb_mutex.lock();
            strncpy(ssh2_password, 
                    QUrl::fromPercentEncoding(this->password.toAscii()).toAscii().data(),
                    sizeof(ssh2_password));
            ret = libssh2_userauth_keyboard_interactive(this->sess,
                                                        this->userName.toAscii().data(), &kbd_callback) ;
            memset(ssh2_password, 0, sizeof(ssh2_password));
            ssh2_kbd_cb_mutex.unlock();
            qDebug()<<"Keyboard interactive :"<<ret;
        }
        if (this->user_canceled == true) {
            // this->connect_status = 2 ;
            libssh2_session_disconnect(this->sess, "");
            libssh2_session_free(this->sess);
            this->piClose(this->sock);
            return CONN_CANCEL;
        }   
    }
    ret = libssh2_userauth_authenticated(this->sess);
    if (ret == 0 ) {
        // this->connect_status = CONN_AUTH_ERROR ;
        qDebug()<<"User auth faild";
        emit connect_state_changed(tr("User faild (Keyboard Interactive)(Password )."));
        return CONN_AUTH_ERROR;
    }
    if (this->user_canceled == true ) {
        // this->connect_status = CONN_CANCEL ;
        libssh2_session_disconnect(this->sess, "");
        libssh2_session_free(this->sess);
        this->piClose(this->sock);
        return CONN_CANCEL;
    }   
    
    emit connect_state_changed(tr("User auth successfully"));

    return 0;
}
void SSHConnection::piClose(int sock)
{
#ifdef WIN32
    ::closesocket(sock);
#else
    ::close(sock);
#endif  
}

int SSHConnection::sshHomePath()
{
    int ret = -1;
    char home_path[PATH_MAX+1] = {0};
    LIBSSH2_SFTP *ssh2_sftp = NULL;

    ssh2_sftp = libssh2_sftp_init(this->sess);
    if (ssh2_sftp == NULL) {
        // this->connect_status = CONN_SFTP_ERROR;
        QString msg;
        char *emsg = 0;
        int  emsg_len = 0;
        libssh2_session_last_error(this->sess, &emsg, &emsg_len, 1);
        qDebug()<<"Init sftp error: "<<emsg;
        if (emsg != 0) {
            msg = QString(emsg);
            free(emsg);
        } else {
            msg = QString(tr("Unknown SFTP error."));
        }
        emit connect_state_changed(tr( "Init sftp error: ") + msg);
        return CONN_SFTP_ERROR;
    }
    Q_ASSERT(ssh2_sftp != NULL);
    char **server_info, **pptr;
    server_info = pptr = libssh2_session_get_remote_info(this->sess);
    printf("Received SFTP Version: %d %s\n", libssh2_sftp_get_version((LIBSSH2_SFTP*)ssh2_sftp), server_info[0]);
    while (*pptr != NULL) {
        free(*pptr); 
        pptr++;
    }
    free(server_info);
    
    ret = libssh2_sftp_realpath((LIBSSH2_SFTP*)ssh2_sftp, ".", home_path, PATH_MAX);
    if (ret != 0) {
        QString msg;
        char *emsg = 0;
        int  emsg_len = 0;
        libssh2_session_last_error(this->sess, &emsg, &emsg_len, 1);
        qDebug()<<"Init sftp error: "<<emsg;
        if (emsg != 0) {
            msg = QString(emsg);
            free(emsg);
        } else {
            msg = QString(tr("Unknown SFTP error."));
        }

        qDebug()<<"realpath : "<<ret
                <<"error code: "<<libssh2_sftp_last_error((LIBSSH2_SFTP*)ssh2_sftp)
                <<"error msg: "<<msg
                <<home_path;
        assert(ret >= 0);
    }
    this->homePath = QString(home_path);
    libssh2_sftp_shutdown(ssh2_sftp);
    // this->connect_status = 0 ;
    
    if (this->user_canceled == true) {
        // this->connect_status = CONN_CANCEL;
        libssh2_session_disconnect(this->sess, "");
        libssh2_session_free(this->sess);
        this->piClose(this->sock);
        return CONN_CANCEL;
    }

    return 0;
}
QString SSHConnection::get_server_env_vars(const char *cmd)
{
    LIBSSH2_CHANNEL *ssh2_channel = 0;
    int rv = -1;
    char buff[1024];
    QString env_output;
    QString uname_output;

    ssh2_channel = libssh2_channel_open_session(this->sess);
    //libssh2_channel_set_blocking(ssh2_channel, 1);
    // rv = libssh2_channel_exec(ssh2_channel, cmd);
    rv = libssh2_channel_shell(ssh2_channel);
    qDebug()<<"SSH2 exec: "<<rv;

    rv = libssh2_channel_write(ssh2_channel, cmd, strlen(cmd));
    qDebug()<<"SSH2 write: "<<rv;
    rv = libssh2_channel_write(ssh2_channel, "\n", 1);
    qDebug()<<"SSH2 write2: "<<rv;
    rv = libssh2_channel_send_eof(ssh2_channel);
  
    memset(buff, 0, sizeof(buff));
    while ((rv = libssh2_channel_read(ssh2_channel, buff, 1000)) > 0) {
        // qDebug()<<"Channel read: "<<rv<<" -->"<<buff;
        env_output += QString(buff);
        memset(buff, 0, sizeof(buff));
    }

    libssh2_channel_close(ssh2_channel);
    libssh2_channel_free(ssh2_channel);

    return env_output;
    return QString();
}
