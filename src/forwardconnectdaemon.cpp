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

#include <assert.h>

#include "libssh2.h"
#include "libssh2_sftp.h"

#include "forwardconnectdaemon.h"


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

ForwardConnectDaemon::ForwardConnectDaemon(QWidget *parent)
 : QWidget(parent)
{
    this->ui_fcd.setupUi(this);
    this->init_custom_menu();
    
    QObject::connect ( this,SIGNAL ( customContextMenuRequested ( const QPoint & ) ),
                       this , SLOT ( slot_custom_ctx_menu ( const QPoint & ) ) );
    QObject::connect ( this->ui_fcd.comboBox,SIGNAL ( customContextMenuRequested ( const QPoint & ) ),
                       this , SLOT ( slot_custom_ctx_menu ( const QPoint & ) ) );
    QObject::connect ( this->ui_fcd.toolButton,SIGNAL ( customContextMenuRequested ( const QPoint & ) ),
                       this , SLOT ( slot_custom_ctx_menu ( const QPoint & ) ) );
    
    this->user_canceled = false;
    this->plink_proc = 0;
    QObject::connect(&this->alive_check_timer, SIGNAL(timeout()),this,SLOT(slot_time_out()));
}


ForwardConnectDaemon::~ForwardConnectDaemon()
{
}

void ForwardConnectDaemon::slot_custom_ctx_menu(const QPoint & pos)
{
    //qDebug() <<__FUNCTION__<<": "<<__LINE__<<":"<< __FILE__;
    this->op_menu->popup(this->mapToGlobal(pos));
}

//call once olny
void ForwardConnectDaemon::init_custom_menu()
{
    QAction * action ;
    
    this->op_menu = new QMenu();
    
    action = new QAction ( tr("&New forward..."),0 );
    this->op_menu->addAction ( action );
    QObject::connect(action, SIGNAL(triggered()),  this, SLOT(slot_new_forward()));
    
    action = new QAction ( tr("&Stop forward..."),0 );
    this->op_menu->addAction ( action );
    QObject::connect(action, SIGNAL(triggered()),  this, SLOT(slot_stop_port_forward()));
}
void ForwardConnectDaemon::slot_stop_port_forward()
{
    this->user_canceled = true;
    this->alive_check_timer.stop();    
    this->plink_id = 0;
    this->plink_proc->kill();
    delete this->plink_proc;
    this->plink_proc = 0;
}

void ForwardConnectDaemon::slot_new_forward()
{
    qDebug() <<__FUNCTION__<<": "<<__LINE__<<":"<< __FILE__;
    if(plink_proc == 0)
    {
        plink_proc = new QProcess(this);
        QObject::connect(plink_proc, SIGNAL(error(QProcess::ProcessError)),this,SLOT(slot_proc_error(QProcess::ProcessError)));

        QObject::connect(plink_proc, SIGNAL(finished ( int , QProcess::ExitStatus  )),this,SLOT(slot_proc_finished ( int , QProcess::ExitStatus  )));
        QObject::connect(plink_proc, SIGNAL(readyReadStandardError ()),this,SLOT(slot_proc_readyReadStandardError ()));
        QObject::connect(plink_proc, SIGNAL(readyReadStandardOutput ()),this,SLOT(slot_proc_readyReadStandardOutput ()));
        QObject::connect(plink_proc, SIGNAL(started ()),this,SLOT(slot_proc_started ()));
        QObject::connect(plink_proc, SIGNAL(stateChanged ( QProcess::ProcessState  )),this,SLOT(slot_proc_stateChanged ( QProcess::ProcessState  )));
    }
    
    QString  program_name = QApplication::applicationDirPath ()+"/plink";//"/home/gzl/nullfxp-svn/src/plink/plink";
    QStringList arg_list ;
    
    //此进程在正常情况下将不断检测，如没有检测到进程存在则重新启动。除非手工停止
    //use kill -SIGINT  , the process can exit normal
    ///plink -ssh -batch -N -v -l webroot -pw xxxxxx -R 8000:0.0.0.0:22 218.244.130.188
    arg_list<<"-ssh";
    arg_list<<"-N";
    arg_list<<"-v";
    arg_list<<"-l"; 
    arg_list<<"webroot";
    arg_list<<"-pw";
    	arg_list<<"xxxxxx";
    arg_list<<"-R";
    arg_list<<"8000:0.0.0.0:22";
    arg_list<<"218.244.130.188";

    this->plink_id = 0;
    plink_proc->start(program_name,arg_list);
    if(!this->alive_check_timer.isActive())
    {
        this->alive_check_timer.setInterval(1000*60*1);
        this->alive_check_timer.start();
    }
}

void ForwardConnectDaemon::slot_proc_error ( QProcess::ProcessError error )
{
	qDebug() <<__FUNCTION__<<": "<<__LINE__<<":"<< __FILE__;
	qDebug() <<error;
	QByteArray ba ;
	ba = plink_proc->readAllStandardError();
	ba += plink_proc->readAllStandardOutput();
	qDebug() <<ba;
    if(error == QProcess::FailedToStart)
    {
        qDebug() <<__FUNCTION__<<": "<<__LINE__<<":"<< __FILE__;
        this->alive_check_timer.stop();
        this->plink_id = 0;
    }
}

void ForwardConnectDaemon::slot_proc_finished ( int exitCode, QProcess::ExitStatus exitStatus )
{
    qDebug() <<__FUNCTION__<<": "<<__LINE__<<":"<< __FILE__;
    qDebug()<<exitCode<<" "<<exitStatus;
    QByteArray ba ;
    ba = plink_proc->readAllStandardError();
    ba += plink_proc->readAllStandardOutput();
    qDebug() <<ba;
    this->plink_id = 0;
    if(! this->user_canceled)
    {
    	qDebug()<<"plink process finished, but not user canceled, restart after 2 second...";
    	this->alive_check_timer.stop();
    	//this->slot_new_forward();
    	QTimer::singleShot(1000*2,this,SLOT(slot_new_forward()) );
	}
}
void ForwardConnectDaemon::slot_proc_readyReadStandardError ()
{
//     qDebug() <<__FUNCTION__<<": "<<__LINE__<<":"<< __FILE__;
    QByteArray ba ;
    ba = plink_proc->readAllStandardError();
//     qDebug() <<ba;
}
void ForwardConnectDaemon::slot_proc_readyReadStandardOutput ()
{
//     qDebug() <<__FUNCTION__<<": "<<__LINE__<<":"<< __FILE__;
    QByteArray ba ;
    ba = plink_proc->readAllStandardOutput();
//     qDebug() <<ba;
}
void ForwardConnectDaemon::slot_proc_started ()
{
    qDebug() <<__FUNCTION__<<": "<<__LINE__<<":"<< __FILE__;
    this->plink_id = this->plink_proc->pid();
}
void ForwardConnectDaemon::slot_proc_stateChanged ( QProcess::ProcessState newState )
{
//     qDebug() <<__FUNCTION__<<": "<<__LINE__<<":"<< __FILE__;
}
void ForwardConnectDaemon::slot_time_out()
{
    qDebug() <<__FUNCTION__<<": "<<__LINE__<<":"<< __FILE__;
    qDebug()<<this->plink_id<<" "<<this->user_canceled<<" "<<QDateTime::currentDateTime();
    if(this->plink_id == 0 && ! this->user_canceled)
    {
        qDebug()<<"plink process disappeared, restart...";
        this->slot_new_forward();
    }
    if(this->user_canceled) this->alive_check_timer.stop();
    else if(!this->alive_check_timer.isActive()) this->alive_check_timer.start();
}