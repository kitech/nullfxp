/* remotehostconnectthread.h --- 
 * 
 * Filename: remotehostconnectthread.h
 * Description: 
 * Author: 刘光照<liuguangzhao@comsenz.com>
 * Maintainer: 
 * Copyright (C) 2000-2008 www.comsenz.com
 * Created: 三  5月 14 15:30:32 2008 (UTC)
 * Version: 
 * Last-Updated: 六  6月 14 22:30:28 2008 (CST)
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

///
///
///
class RemoteHostConnectThread : public QThread
{
Q_OBJECT
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
