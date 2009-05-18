// forwardconnectdaemon.h --- 
// 
// Author: liuguangzhao
// Copyright (C) 2007-2010 liuguangzhao@users.sf.net
// URL: http://www.qtchina.net http://nullget.sourceforge.net
// Created: 2007-05-18 22:11:37 +0800
// Last-Updated: 
// Version: $Id$
// 
#ifndef FORWARDCONNECTDAEMON_H
#define FORWARDCONNECTDAEMON_H

#include <QtCore>
#include <QtGui>
#include <QWidget>

#include "ui_forwardconnectdaemon.h"

class ForwardDebugWindow;
class ForwardConnectInfoDialog;

class ForwardProcessDaemon: public QThread
{
    Q_OBJECT;
public:
    ForwardProcessDaemon(QObject *parent = 0)
        : QThread(parent)
    {
    }
    ~ForwardProcessDaemon(){};
    void run();
    int type;
    QString response; 
};

class ForwardList: public QObject
{
    Q_OBJECT;
public:
    ForwardList(){
        this->fp_thread = new ForwardProcessDaemon();
        this->plink_proc = new QProcess();
        this->ps_proc = new QProcess();
        this->user_canceled = false;
        this->ps_exist = 0;
    }
    ~ForwardList(){
        delete this->fp_thread;
        delete this->plink_proc;
        delete this->ps_proc;
    }
    
    ////////////////////////
    QString host;
    QString user_name;
    QString passwd;
    QString remote_listen_port;
    QString forward_local_port;
    QString remote_home_path;
    int status;
    int ps_exist;
    QTimer alive_check_timer;
    QProcess *plink_proc;
    Q_PID plink_id ;
    QProcess *ps_proc;
    Q_PID ps_id ;
    ForwardProcessDaemon *fp_thread;
    bool user_canceled ;
};


/**
   @author liuguangzhao <liuguangzhao@users.sf.net>
*/
class ForwardConnectDaemon : public QWidget
{
    Q_OBJECT;
public:
    ForwardConnectDaemon(QWidget *parent = 0);

    ~ForwardConnectDaemon();
    
    enum {DBG_ALL, DBG_INFO, DBG_DEBUG, DBG_ERROR};
    
private slots:
    void slot_custom_ctx_menu ( const QPoint & pos );
    void slot_new_forward();
    void slot_start_forward(ForwardList *fl);
    void slot_proc_error ( QProcess::ProcessError error );
    void slot_proc_finished ( int exitCode, QProcess::ExitStatus exitStatus );
    void slot_proc_readyReadStandardError ();
    void slot_proc_readyReadStandardOutput ();
    void slot_proc_started ();
    void slot_proc_stateChanged ( QProcess::ProcessState newState );
    void slot_time_out();
    void slot_stop_port_forward();
    void slot_show_debug_window();
    void slot_forward_index_changed(int index);
    
private:
    void init_custom_menu();
    ForwardList *get_forward_list_by_proc(QObject *proc_obj, int *which);
    ForwardList *get_forward_list_by_timer(QObject *proc_obj);
    ForwardList *get_forward_list_by_serv_info();
    
private:
    Ui::ForwardConnectDaemon ui_fcd;
    QMenu *op_menu;
    ForwardDebugWindow *fdw ;
    ForwardConnectInfoDialog *info_dlg;
    
    int connect_status;
    void *ssh2_sess;
    int ssh2_sock;
    void *ssh2_sftp;
    
    QVector<ForwardList*> forward_list;
    
    QStringListModel *host_model;
    
signals:
    void log_debug_message(QString key, int level, QString msg);
};

#endif
