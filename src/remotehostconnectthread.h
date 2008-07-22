/* remotehostconnectthread.h --- 
 * 
 * Filename: remotehostconnectthread.h
 * Description: 
 * Author: 刘光照<liuguangzhao@users.sf.net>
 * Maintainer: 
 * Copyright (C) 2007-2008 liuguangzhao <liuguangzhao@users.sf.net>
 * http://www.qtchina.net
 * http://nullget.sourceforge.net
 * Created: 二  7月 22 21:11:46 2008 (CST)
 * Version: 
 * Last-Updated: 二  7月 22 21:11:51 2008 (CST)
 *           By: 刘光照<liuguangzhao@users.sf.net>
 *     Update #: 1
 * URL: 
 * Keywords: 
 * Compatibility: 
 * 
 */

/* Commentary: 
 * 
 * 
 * 
 */

/* Change log:
 * 
 * 
 */

#ifndef REMOTEHOSTCONNECTTHREAD_H
#define REMOTEHOSTCONNECTTHREAD_H

#include <QtCore>
#include <QThread>

#include "libssh2.h"
#include "libssh2_sftp.h"

class ConnectionInfo : public QObject
{
    Q_OBJECT;
public:
ConnectionInfo(QObject *parent = 0) : QObject(parent) {}
    ~ConnectionInfo(){}
public:
    QString userName;
    QString password;   //存储的密码为url编码过的
    QString decodedPassword;
    QString hostName ;
    short   port;
    QString pubkeyPath;
    QString homePath;
    
    LIBSSH2_SESSION * sshSess;
    LIBSSH2_SFTP * sshSftp ;
    int sshSock;
};
///
///
///
class RemoteHostConnectThread : public QThread
{
    Q_OBJECT;
public:
    enum {CONN_OK=0,CONN_REFUSE,CONN_CANCEL,CONN_OTHER,CONN_RESOLVE_ERROR,CONN_SESS_ERROR,CONN_AUTH_ERROR,CONN_SFTP_ERROR,CONN_EXEC_ERROR};
    RemoteHostConnectThread(QString user_name, QString password, QString host_name, 
                            short port, QString pubkey, QObject* parent=0);

    ~RemoteHostConnectThread();

    virtual void run();
    
    void do_init() ;
    void diconnect_ssh_connection();
    std::string get_user_home_path () ;
    
    QString get_host_name () ;
    QString get_user_name () ;
    QString get_password () ;
    short   get_port();
    QString get_pubkey();
    void * get_ssh2_sess () ;
    int get_ssh2_sock () ;
    int get_connect_status();
    QString get_status_desc(int status);
    void set_user_canceled();
    
    signals:
        void  connect_state_changed( QString state_desc );
        
    private:
        QString user_name;
        QString password;   //存储的密码为url编码过的
        QString decoded_password;
        QString host_name ;
        short   port;
        QString pubkey_path;
        int connect_status;
        bool user_canceled;
        
        std::string user_home_path ;
        
        void * ssh2_sess;
        int ssh2_sock;
        void * ssh2_sftp ;
        
    private slots:
        void slot_finished()   ;
 private:
	QString get_server_env_vars(char *cmd);

    signals:
        void connect_finished( int status , void * ssh2_sess , int ssh2_sock /*, void * ssh2_sftp*/ );
};

#endif
