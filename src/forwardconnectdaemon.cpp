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
#include "forwarddebugwindow.h"
#include "forwardconnectinfodialog.h"

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
    
    QObject::connect(this->ui_fcd.comboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(slot_forward_index_changed(int)));
    
    fdw = 0;
    this->host_model = new QStringListModel();
}


ForwardConnectDaemon::~ForwardConnectDaemon()
{
}

void ForwardConnectDaemon::slot_custom_ctx_menu(const QPoint & pos)
{
    //qDebug() <<__FUNCTION__<<": "<<__LINE__<<":"<< __FILE__;
    //TODO 按情况让某些菜单变灰色
    //if(this->ui_fcd.comboBox.count() == 0) 
        
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
    
    action = new QAction("",0);
    action->setSeparator(true);
    this->op_menu->addAction ( action );
    
    action = new QAction ( tr("Show &Debug Window"),0 );
    this->op_menu->addAction ( action );
    QObject::connect(action, SIGNAL(triggered()),  this, SLOT(slot_show_debug_window()));
    
}
void ForwardConnectDaemon::slot_stop_port_forward()
{
    //算法说明：
    //通过下拉框的内容，找到它所对应的ForwardList 对象
    ForwardList * fl = 0;
    
    fl = this->get_forward_list_by_serv_info();
    fl->user_canceled = true;
    fl->alive_check_timer.stop();
    fl->plink_id = 0;
    fl->plink_proc->kill();
    fl->ps_id = 0;
    fl->ps_proc->kill();
    
    //从下拉框中删掉这一条
    this->ui_fcd.comboBox->removeItem(this->ui_fcd.comboBox->currentIndex());
}

void ForwardConnectDaemon::slot_new_forward()
{
    qDebug() <<__FUNCTION__<<": "<<__LINE__<<":"<< __FILE__;
    
    int dlg_res = 0 ;
    ForwardConnectInfoDialog * info_dlg;
    info_dlg = new ForwardConnectInfoDialog(this);
    dlg_res = info_dlg->exec();
    if(dlg_res == QDialog::Rejected)
    {
        //
        delete info_dlg ;
        return;
    }
    ForwardList *fl = new ForwardList();
    this->forward_list.append(fl);
    info_dlg->get_forward_info(fl->host, fl->user_name, fl->passwd, fl->remote_listen_port,fl->forward_local_port);
    delete info_dlg;
    
    this->ui_fcd.comboBox->addItem(fl->host+ fl->user_name+ fl->passwd+ fl->remote_listen_port+fl->forward_local_port);
    this->ui_fcd.comboBox->setToolTip(fl->host+ fl->user_name+ fl->passwd+ fl->remote_listen_port+fl->forward_local_port);
    //
    QObject::connect(fl->plink_proc, SIGNAL(error(QProcess::ProcessError)),this,SLOT(slot_proc_error(QProcess::ProcessError)));

    QObject::connect(fl->plink_proc, SIGNAL(finished ( int , QProcess::ExitStatus  )),this,SLOT(slot_proc_finished ( int , QProcess::ExitStatus  )));
    QObject::connect(fl->plink_proc, SIGNAL(readyReadStandardError ()),this,SLOT(slot_proc_readyReadStandardError ()));
    QObject::connect(fl->plink_proc, SIGNAL(readyReadStandardOutput ()),this,SLOT(slot_proc_readyReadStandardOutput ()));
    QObject::connect(fl->plink_proc, SIGNAL(started ()),this,SLOT(slot_proc_started ()));
    QObject::connect(fl->plink_proc, SIGNAL(stateChanged ( QProcess::ProcessState  )),this,SLOT(slot_proc_stateChanged ( QProcess::ProcessState  )));
    QObject::connect(&fl->alive_check_timer,SIGNAL(timeout()), this, SLOT(slot_time_out()));
    
    QObject::connect(fl->ps_proc, SIGNAL(error(QProcess::ProcessError)),this,SLOT(slot_proc_error(QProcess::ProcessError)));
    QObject::connect(fl->ps_proc, SIGNAL(finished ( int , QProcess::ExitStatus  )),this,SLOT(slot_proc_finished ( int , QProcess::ExitStatus  )));
    QObject::connect(fl->ps_proc, SIGNAL(readyReadStandardError ()),this,SLOT(slot_proc_readyReadStandardError ()));
    QObject::connect(fl->ps_proc, SIGNAL(readyReadStandardOutput ()),this,SLOT(slot_proc_readyReadStandardOutput ()));
    QObject::connect(fl->ps_proc, SIGNAL(started ()),this,SLOT(slot_proc_started ()));
    QObject::connect(fl->ps_proc, SIGNAL(stateChanged ( QProcess::ProcessState  )),this,SLOT(slot_proc_stateChanged ( QProcess::ProcessState  )));
    
    this->slot_start_forward(fl);

    return; //depcrated code
//     if(plink_proc == 0)
//     {
//         plink_proc = new QProcess(this);
//         QObject::connect(plink_proc, SIGNAL(error(QProcess::ProcessError)),this,SLOT(slot_proc_error(QProcess::ProcessError)));
// 
//         QObject::connect(plink_proc, SIGNAL(finished ( int , QProcess::ExitStatus  )),this,SLOT(slot_proc_finished ( int , QProcess::ExitStatus  )));
//         QObject::connect(plink_proc, SIGNAL(readyReadStandardError ()),this,SLOT(slot_proc_readyReadStandardError ()));
//         QObject::connect(plink_proc, SIGNAL(readyReadStandardOutput ()),this,SLOT(slot_proc_readyReadStandardOutput ()));
//         QObject::connect(plink_proc, SIGNAL(started ()),this,SLOT(slot_proc_started ()));
//         QObject::connect(plink_proc, SIGNAL(stateChanged ( QProcess::ProcessState  )),this,SLOT(slot_proc_stateChanged ( QProcess::ProcessState  )));
//     }
    
//     QString  program_name = QApplication::applicationDirPath ()+"/plink";//"/home/gzl/nullfxp-svn/src/plink/plink";
//     QStringList arg_list ;
//     
//     //此进程在正常情况下将不断检测，如没有检测到进程存在则重新启动。除非手工停止
//     //use kill -SIGINT  , the process can exit normal
//     ///plink -ssh -batch -N -v -l webroot -pw xxxxxx -R 8000:0.0.0.0:22 218.244.130.188
//     arg_list<<"-ssh";
//     arg_list<<"-N";
//     arg_list<<"-v";
//     arg_list<<"-l"; 
//     arg_list<<"webroot";
//     arg_list<<"-pw";
//     	arg_list<<"xxxxxxx";
//     arg_list<<"-R";
//     arg_list<<"8000:0.0.0.0:22";
//     arg_list<<"218.244.130.188";
// 
//     this->plink_id = 0;
//     plink_proc->start(program_name,arg_list);
//     if(!this->alive_check_timer.isActive())
//     {
//         this->alive_check_timer.setInterval(1000*60*1);
//         this->alive_check_timer.start();
//     }
}

void ForwardConnectDaemon::slot_start_forward(ForwardList * fl)
{
    QString  program_name = QApplication::applicationDirPath ()+"/plink";//"/home/gzl/nullfxp-svn/src/plink/plink";
    #ifdef WIN32
    program_name += ".exe";
    #endif
    QStringList arg_list ;
    
    //此进程在正常情况下将不断检测，如没有检测到进程存在则重新启动。除非手工停止
    //use kill -SIGINT  , the process can exit normal
    ///plink -ssh -batch -N -v -l webroot -pw xxxxxx -R 8000:0.0.0.0:22 218.244.130.188
    //>plink -v -N -C -l webroot -R 8081:localhost:80 qtchina.3322.org
    arg_list<<"-ssh";
    arg_list<<"-N";
    arg_list<<"-v";
    arg_list<<"-l"; 
//     arg_list<<"webroot";
    arg_list<<fl->user_name;
    arg_list<<"-pw";
//     arg_list<<"webadmin";
    arg_list<<fl->passwd;
    arg_list<<"-R";
//     arg_list<<"6000:0.0.0.0:22";
    #ifdef WIN32
    arg_list<<(fl->remote_listen_port+":localhost:"+fl->forward_local_port);
    #else
    arg_list<<(fl->remote_listen_port+":0.0.0.0:"+fl->forward_local_port);
    #endif
//     arg_list<<"218.244.130.188";
    arg_list<<fl->host;

		//qDebug()<<arg_list;

    fl->plink_id = 0;
    fl->plink_proc->start(program_name,arg_list);
    fl->alive_check_timer.setInterval(1000*20*1);
    if(!fl->alive_check_timer.isActive())
    {
        fl->alive_check_timer.start();
    }
}

void ForwardConnectDaemon::slot_proc_error ( QProcess::ProcessError error )
{
// 	qDebug() <<__FUNCTION__<<": "<<__LINE__<<":"<< __FILE__;
// 	qDebug() <<error;
	QByteArray ba ;
	//ba = plink_proc->readAllStandardError();
	//ba += plink_proc->readAllStandardOutput();
 	//qDebug() <<ba;
    if(error == QProcess::FailedToStart)
    {
//         qDebug() <<__FUNCTION__<<": "<<__LINE__<<":"<< __FILE__;
        //this->alive_check_timer.stop();
        //this->plink_id = 0;
    }
}

void ForwardConnectDaemon::slot_proc_finished ( int exitCode, QProcess::ExitStatus exitStatus )
{
    qDebug() <<__FUNCTION__<<": "<<__LINE__<<":"<< __FILE__;
//     qDebug()<<exitCode<<" "<<exitStatus;
    QByteArray ba ;
    ForwardList * fl = 0;
    int which = 0 ;

    fl = this->get_forward_list_by_proc(sender(), &which);
    fl->plink_id = fl->plink_proc->pid();
    if(which == 2)
    {
        if(fl->ps_exist == 1)//没有找到相关端口, 重新启动
        {
            fl->alive_check_timer.stop();
            fl->plink_id = 0;
            fl->plink_proc->kill();
        }
    }
    if(which == 1)
    {
        fl->plink_id = 0;
        if(! fl->user_canceled)
        {
            qDebug()<<"plink process finished, but not user canceled, restart after 2 second...";
            fl->alive_check_timer.stop();
            fl->alive_check_timer.setInterval(1000*2*1);
            fl->alive_check_timer.start();
        }
    }
    //ba = plink_proc->readAllStandardError();
    //ba += plink_proc->readAllStandardOutput();
//     qDebug() <<ba;
    //this->plink_id = 0;
//     if(! this->user_canceled)
//     {
//     	qDebug()<<"plink process finished, but not user canceled, restart after 2 second...";
//     	//this->alive_check_timer.stop();
//     	//this->slot_new_forward();
//     	QTimer::singleShot(1000*2,this,SLOT(slot_new_forward()) );
// 	}
}
void ForwardConnectDaemon::slot_proc_readyReadStandardError ()
{
//     qDebug() <<__FUNCTION__<<": "<<__LINE__<<":"<< __FILE__;
    ForwardList * fl = 0;
    QProcess *proc ;
    int which = 0;
    QByteArray ba ;

    fl = this->get_forward_list_by_proc(sender(), &which);
    proc = (QProcess*)sender();
    ba = proc->readAllStandardError();
    //qDebug() <<ba;
}
void ForwardConnectDaemon::slot_proc_readyReadStandardOutput ()
{
    qDebug() <<__FUNCTION__<<": "<<__LINE__<<":"<< __FILE__;
    ForwardList * fl = 0;
    int which = 0 ;
    QProcess *proc ;
    QByteArray ba ;
    QStringList lines;
    QStringList fields;

    fl = this->get_forward_list_by_proc(sender(),&which);
    proc = (QProcess*)sender();
    ba = proc->readAllStandardOutput();
    //qDebug() <<ba;
    if(which == 2)
    {
        //解析输出结果
        lines = QString(ba.data()).split("\n");
        //qDebug()<<lines;
        for(int i=0;i<lines.count();i++)
        {
            fields = lines.at(i).split(" ", QString::SkipEmptyParts);
            //qDebug()<<fields;
            if(fields.count() > 0 && fields.at(fields.count()-1) == QString("LISTEN"))
            {
                qDebug()<<"found expected listen port.";
                fl->ps_exist = 2;
            }
        }
    }
    emit log_debug_message(QString("%1:%2").arg(fl->host).arg(fl->remote_listen_port), DBG_INFO, QString(ba));
}
void ForwardConnectDaemon::slot_proc_started ()
{
    qDebug() <<__FUNCTION__<<": "<<__LINE__<<":"<< __FILE__;
    ForwardList * fl = 0;
    int which = 0 ;

    fl = this->get_forward_list_by_proc(sender(), &which);
    fl->plink_id = fl->plink_proc->pid();
    if(which == 2)
    {
        fl->ps_exist = 1;
    }
}
void ForwardConnectDaemon::slot_proc_stateChanged ( QProcess::ProcessState newState )
{
//     qDebug() <<__FUNCTION__<<": "<<__LINE__<<":"<< __FILE__;
}
void ForwardConnectDaemon::slot_time_out()
{
    qDebug() <<__FUNCTION__<<": "<<__LINE__<<":"<< __FILE__;
    //qDebug()<<this->plink_id<<" "<<this->user_canceled<<" "<<QDateTime::currentDateTime();
    //这种检测不够，还要使用plink连接到远程服务器查看相关端口是否能用
    //like this : plink -l webroot -pw xxxxxxx xxx.xxx.xxx.xxx netstat -ant|grep 8000
    ForwardList * fl = 0;
    QProcess *plink_proc ;
    QProcess *ps_proc ;
    Q_PID  plink_id;
    Q_PID  ps_id;
    
    fl = this->get_forward_list_by_timer(sender());
    plink_proc = fl->plink_proc;
    ps_proc = fl->ps_proc;
    
    if(fl->plink_id == 0 && ! fl->user_canceled)
    {
        qDebug()<<"plink process disappeared, restart...";
        //this->slot_new_forward();
        this->slot_start_forward(fl);
    }else{
        qDebug()<<"plink process exist, checking port status...";
    	//执行远程端口检测命令,使用新进程方式
    	//有两种方式，第一种使用plink进程实现此功能; 第二种使用libssh2库执行远程命令
        QString  program_name = QApplication::applicationDirPath ()+"/plink";//"/home/gzl/nullfxp-svn/src/plink/plink";
        QStringList arg_list ;
                
        //此进程在正常情况下将不断检测，如没有检测到进程存在则重新启动。除非手工停止
        //use kill -SIGINT  , the process can exit normal
        //plink -l webroot -pw xxxxxxx xxx.xxx.xxx.xxx netstat -ant|grep 8000
//         arg_list<<"-ssh";
//         arg_list<<"-N";
        arg_list<<"-v";
        arg_list<<"-l"; 
//         arg_list<<"webroot";
        arg_list<<fl->user_name;
        arg_list<<"-pw";
            arg_list<<fl->passwd;
//         arg_list<<"-R";
//         arg_list<<"8000:0.0.0.0:22";
//         arg_list<<"218.244.130.188";
        arg_list<<fl->host;
        arg_list<<"netstat -ant|grep \"127.0.0.1:\""+fl->remote_listen_port;
        fl->ps_proc->start(program_name, arg_list);
	}
    //if(this->user_canceled) this->alive_check_timer.stop();
    //else if(!this->alive_check_timer.isActive()) this->alive_check_timer.start();
}
void ForwardConnectDaemon::slot_show_debug_window()
{
    if(this->fdw == 0)
    {
        this->fdw = new ForwardDebugWindow(this);
        QObject::connect(this, SIGNAL(log_debug_message(QString, int , QString)),
                         this->fdw, SLOT(slot_log_debug_message(QString, int, QString)));
    }
    if(!this->fdw->isVisible())
        this->fdw->show();
}

ForwardList * ForwardConnectDaemon::get_forward_list_by_proc(QObject *proc_obj, int *which)
{
    ForwardList * fl = 0;
    *which = 0;
    
    for(int i = 0 ; i < this->forward_list.count(); i ++)
    {
        fl = this->forward_list.at(i);
        if( fl->ps_proc == proc_obj )
        {
            *which = 2;
            break;
        }
        else if(fl->plink_proc == proc_obj)
        {
            *which = 1;
            break;
        }
        fl = 0;
    }
    assert(fl != 0);
    return fl;
}
ForwardList * ForwardConnectDaemon::get_forward_list_by_serv_info()
{
    ForwardList * fl = 0;
    
    QString item_text;
    QString fl_serv_digest;
    
    item_text = this->ui_fcd.comboBox->currentText();
    
    for(int i = 0 ; i < this->forward_list.count(); i ++)
    {
        fl = this->forward_list.at(i);
        fl_serv_digest = fl->host+ fl->user_name+ fl->passwd+ fl->remote_listen_port+fl->forward_local_port;
        if(fl_serv_digest == item_text)
            break;
        fl = 0;
    }
    assert(fl != 0);
    return fl;
}
ForwardList * ForwardConnectDaemon::get_forward_list_by_timer(QObject *timer_obj)
{
    ForwardList * fl = 0;
    
    QString item_text;
    QString fl_serv_digest;
    
    item_text = this->ui_fcd.comboBox->currentText();
    
    for(int i = 0 ; i < this->forward_list.count(); i ++)
    {
        fl = this->forward_list.at(i);
        if(timer_obj == &fl->alive_check_timer)
            break;
        fl = 0;
    }
    assert(fl != 0);
    return fl;
}

void ForwardConnectDaemon::slot_forward_index_changed(int index)
{
    QString item_text;
    
    item_text = this->ui_fcd.comboBox->itemText(index);
    this->ui_fcd.comboBox->setToolTip(item_text);
}

void ForwardProcessDaemon::run()
{
    
}

