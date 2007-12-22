/***************************************************************************
 *   Copyright (C) 2007 by liuguangzhao   *
 *   liuguangzhao@users.sourceforge.net   *
 *
 *   http://www.qtchina.net                                                *
 *   http://nullget.sourceforge.net                                        *
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
    Q_OBJECT
    public:
        ForwardProcessDaemon(QObject * parent = 0):QThread(parent)
        {
        }
        ~ForwardProcessDaemon(){};
        void run();
        int type;
        QString response; 
};

class ForwardList: public QObject
{
    Q_OBJECT
    public:
        ForwardList(){
            this->fp_thread = new ForwardProcessDaemon();
            this->plink_proc = new QProcess();
            this->ps_proc = new QProcess();
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
        QTimer alive_check_timer;
        QProcess * plink_proc;
        Q_PID plink_id ;
        QProcess * ps_proc;
        Q_PID ps_id ;
        ForwardProcessDaemon * fp_thread;
};


/**
	@author liuguangzhao <liuguangzhao@users.sourceforge.net>
*/
class ForwardConnectDaemon : public QWidget
{
Q_OBJECT
public:
    ForwardConnectDaemon(QWidget *parent = 0);

    ~ForwardConnectDaemon();
    private slots:
		void slot_custom_ctx_menu ( const QPoint & pos );
		void slot_new_forward();
		void slot_proc_error ( QProcess::ProcessError error );
        void slot_proc_finished ( int exitCode, QProcess::ExitStatus exitStatus );
        void slot_proc_readyReadStandardError ();
        void slot_proc_readyReadStandardOutput ();
        void slot_proc_started ();
        void slot_proc_stateChanged ( QProcess::ProcessState newState );
        void slot_time_out();
        void slot_stop_port_forward();
        void slot_show_debug_window();
        
        
    private:
        void init_custom_menu();
        ForwardList * get_forward_list_by_proc(int which);
        
    private:
        Ui::ForwardConnectDaemon ui_fcd;
        QMenu *op_menu;
        //QTimer alive_check_timer;
        ForwardDebugWindow * fdw ;
        ForwardConnectInfoDialog * info_dlg;

        int connect_status;
        bool user_canceled;
        void * ssh2_sess;
        int ssh2_sock;
        void * ssh2_sftp ;
        
        QProcess * plink_proc;
        QProcess * plink_proc_cmd;
        int listen_port;
        int forward_port;
        QString server_ip;
        QString listen_ip;
        QString user_name;
        QString password;
        Q_PID plink_id ;
        
        QVector<ForwardList*> forward_list;
};

#endif
