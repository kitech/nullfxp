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
        
    private:
        void init_custom_menu();
    private:
        Ui::ForwardConnectDaemon ui_fcd;
        QMenu *op_menu;
        QTimer alive_check_timer;
        
//         std::string user_name;
//         std::string password;
//         std::string host_name ;
//         std::string user_home_path ;
        int connect_status;
        bool user_canceled;
        void * ssh2_sess;
        int ssh2_sock;
        void * ssh2_sftp ;
        
        QProcess * plink_proc;
        int listen_port;
        int forward_port;
        QString server_ip;
        QString listen_ip;
        QString user_name;
        QString password;
};

#endif
