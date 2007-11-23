/***************************************************************************
 *   Copyright (C) 2007 by liuguangzhao   *
 *   liuguangzhao@users.sourceforge.net   *
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

#ifdef WIN32
#include <winsock2.h>
#else
#include <sys/socket.h>
#include <wait.h>
#endif

#include <QtNetwork>

#include <QCoreApplication>

#include "nullfxp.h"


#include "localview.h"
#include "remoteview.h"
#include "progressdialog.h"
#include "remotehostconnectingstatusdialog.h"
#include "remotehostquickconnectinfodialog.h"
#include "remotehostconnectthread.h"


//////////////////////////

NullFXP::NullFXP ( QWidget * parent , Qt::WindowFlags flags )
		: QMainWindow ( parent ,  flags )
{
	this->mUIMain.setupUi ( this );
    this->setWindowIcon(QIcon(":/icons/nullget-1.png") ); 
    //////////////////////////
	central_splitter_widget  = new QSplitter ( Qt::Vertical );

	//
	mdiArea = new QMdiArea;
    mdiArea->setWindowIcon(QIcon(":/icons/nullget-2.png") ); 
	QObject::connect ( this->mUIMain.actionTransfer_queue,SIGNAL ( triggered ( bool ) ),
	                   this,SLOT ( slot_show_transfer_queue ( bool ) ) );
	QObject::connect ( this->mUIMain.actionShow_log,SIGNAL ( triggered ( bool ) ),
	                   this,SLOT ( slot_show_fxp_command_log ( bool ) ) );

	QObject::connect ( this->mUIMain.actionCascade_window,SIGNAL ( triggered() ),this,SLOT ( slot_cascade_sub_windows() ) );
	QObject::connect ( this->mUIMain.actionTile_window,SIGNAL ( triggered() ),this,SLOT ( slot_tile_sub_windows() ) );

	transfer_queue_list_view = new QListView();

	central_splitter_widget->addWidget ( mdiArea );
	central_splitter_widget->addWidget ( transfer_queue_list_view );
    QSizePolicy sp;
    sp.setVerticalPolicy(QSizePolicy::Ignored);
    this->transfer_queue_list_view->setSizePolicy( sp ) ;
    this->transfer_queue_list_view->setVisible(false );
    
	setCentralWidget ( central_splitter_widget );

	windowMapper = new QSignalMapper ( this );

	///////////////////////
	//QObject::connect(this->mUIMain.pushButton,SIGNAL(clicked()),this,SLOT(test()));
	//QObject::connect(this->mUIMain.pushButton_do_init,SIGNAL(clicked()),this,SLOT(do_init()));
	//QObject::connect(this->mUIMain.pushButton_do_ls,SIGNAL(clicked()),this,SLOT(do_ls()));
	//QObject::connect ( this->mUIMain.actionInit_dir_view, SIGNAL ( triggered() ) ,
	//                   this, SLOT ( local_init_dir_view() ) );
	QObject::connect ( this->mUIMain.actionConnect, SIGNAL ( triggered() ) ,
	                   this, SLOT ( connect_to_remote_host() ) );
	QObject::connect ( this->mUIMain.actionDisconnect,SIGNAL ( triggered() ),
	                   this,SLOT ( slot_disconnect_from_remote_host() ) );

	localView = new LocalView();

	mdiArea->addSubWindow ( localView );

	QObject::connect ( localView,SIGNAL ( new_upload_requested ( QStringList ) ),
	                   this,SLOT ( slot_new_upload_requested ( QStringList ) ) );

	//
	QObject::connect ( this->mUIMain.action_Local_Window,SIGNAL ( triggered() ),
	                   this,SLOT ( slot_show_local_view() ) );
	QObject::connect ( this->mUIMain.action_Remote_Window,SIGNAL ( triggered() ),
	                   this,SLOT ( slot_show_remote_view() ) );

	//////////////
	about_nullfxp_dialog = new AboutNullFXP ( this );
	QObject::connect ( this->mUIMain.actionAbout_NullFXP,SIGNAL ( triggered() ),
	                   this,SLOT ( slot_about_nullfxp() ) );
	QObject::connect ( this->mUIMain.actionAbout_Qt,SIGNAL ( triggered() ),
	                   qApp,SLOT ( aboutQt() ) );

	//启动主界面大小调整
	//this->slot_tile_sub_windows();

	this->central_splitter_widget->setStretchFactor ( 0,3 );
	this->central_splitter_widget->setStretchFactor ( 1,1 );

    //启动连接对话框
    this->show();
    //根据当前屏幕大小调整界面的大小。
    QDesktopWidget * dw = new QDesktopWidget();
    //qDebug()<<dw->screenGeometry();
    this->resize(dw->screenGeometry().width()*4/5, dw->screenGeometry().height()*4/5) ;
    delete dw ;
    //调整本地目录树窗口的大小
	//QList<QMdiSubWindow *> mdiSubWindow = mdiArea->subWindowList();
	//qDebug()<<" mdi sub window count :"<< mdiSubWindow.count();
    QMdiSubWindow * local_sub_win = mdiArea->subWindowList().at(0);
    local_sub_win->setGeometry( local_sub_win->x(),local_sub_win->y(), mdiArea->width()/2,  mdiArea->height()*18/19 );
    
    this->connect_to_remote_host();

}

NullFXP::~NullFXP()
{}
void NullFXP::slot_about_nullfxp()
{
	this->about_nullfxp_dialog->setVisible ( true );
}

void NullFXP::connect_to_remote_host()
{
	qDebug() <<__FUNCTION__<<": "<<__LINE__<<":"<< __FILE__;

	QString username ;
	QString password ;
	QString remoteaddr ;

	//提示输入远程主机信息
	this->quick_connect_info_dailog = new RemoteHostQuickConnectInfoDialog ( this );
	if ( this->quick_connect_info_dailog->exec() == QDialog::Accepted )
	{
		username = this->quick_connect_info_dailog->get_user_name();
		password = this->quick_connect_info_dailog->get_password();
		remoteaddr = this->quick_connect_info_dailog->get_host_name();

		delete this->quick_connect_info_dailog;this->quick_connect_info_dailog=0;

        this->connect_status_dailog = new RemoteHostConnectingStatusDialog ( username,remoteaddr,this, Qt::Dialog );
        QObject::connect(this->connect_status_dailog,SIGNAL(cancel_connect()),
                         this,SLOT(slot_cancel_connect()) );
		//this->localView->set_sftp_connection ( &theconn );
		remote_conn_thread = new RemoteHostConnectThread (
		                         username,  password,remoteaddr ) ;
		QObject::connect ( this->remote_conn_thread , SIGNAL ( connect_finished ( int,void * , int /* , void **/ ) ),
		                   this, SLOT ( slot_connect_remote_host_finished ( int ,void * ,int /* , void **/ ) ) );

        QObject::connect(remote_conn_thread , SIGNAL(connect_state_changed(QString)),
                         connect_status_dailog,SLOT(slot_connect_state_changed(QString)));
        
        this->remote_conn_thread->start();        
		this->connect_status_dailog->exec();
	}
	else
	{
		qDebug() <<"user canceled ...";
	}

}

void NullFXP::slot_disconnect_from_remote_host()
{
	qDebug() <<__FUNCTION__<<": "<<__LINE__<<":"<< __FILE__;

    RemoteView * remote_view = this->get_top_most_remote_view() ;
    
    if( remote_view == 0 )
    {
        //do nothing
    }
    else
    {
        qDebug()<< " disconnect : "<< remote_view->windowTitle() ;
        QList<QMdiSubWindow *> sub_window_list = this->mdiArea->subWindowList(QMdiArea::StackingOrder);
        int sub_wnd_count = sub_window_list.count() ;
    
        for( sub_wnd_count = sub_wnd_count -1 ;  sub_wnd_count >= 0  ; sub_wnd_count -- )
        {
            if( sub_window_list.at( sub_wnd_count )->widget() != this->localView 
                && sub_window_list.at( sub_wnd_count )->widget()->objectName()=="rv" )
            {
                sub_window_list.at( sub_wnd_count )->close();
                break ;
            }
        }
    }
}

void NullFXP::slot_connect_remote_host_finished ( int status,void * ssh2_sess , int ssh2_sock /* , void * ssh2_sftp*/ )
{
    RemoteHostConnectThread * conn_thread = static_cast< RemoteHostConnectThread*>(sender()) ;
    
	qDebug() <<__FUNCTION__<<": "<<__LINE__<<":"<< __FILE__;
	if ( status == 0 )
	{
        RemoteView * remote_view = new RemoteView(this->mdiArea , this->localView);
        
        mdiArea->addSubWindow ( remote_view );
        QMdiSubWindow * local_sub_win = mdiArea->subWindowList(QMdiArea::CreationOrder).at(mdiArea->subWindowList(QMdiArea::CreationOrder).count()-1);
        
        remote_view->slot_custom_ui_area();
        //remote_view->show();
        local_sub_win->show();
        //调整本地目录树窗口的大小
        //QList<QMdiSubWindow *> mdiSubWindow = mdiArea->subWindowList();
        //qDebug()<<" mdi sub window count :"<< mdiSubWindow.count();        
        //local_sub_win->setGeometry( local_sub_win->x(),local_sub_win->y(), mdiArea->width()/2,  mdiArea->height()*18/19 );
        local_sub_win->resize(mdiArea->width()/2, mdiArea->height()*18/19) ;
        
        remote_view->set_ssh2_handler(ssh2_sess/*,ssh2_sftp*/ ,ssh2_sock);
        remote_view->set_user_home_path ( this->remote_conn_thread->get_user_home_path() );
        remote_view->set_host_info(conn_thread->get_host_name(),
                                   conn_thread->get_user_name(),
                                           conn_thread->get_password() );
        //初始化远程目录树        
        remote_view->i_init_dir_view (  );
	}
	else if( status == 2 ) {   //use canceled connection
        qDebug()<<"user canceled connecting";
    }else
	{
		//assert ( 1==2 );
        //this->connect_status_dailog->setVisible(false);
        QMessageBox::critical(this,tr("Connect Error:"),tr("Check user name and password , retry again") );
	}
	this->connect_status_dailog->accept();
	delete this->connect_status_dailog ;
	this->connect_status_dailog = 0 ;
    delete this->remote_conn_thread ;
    this->remote_conn_thread = 0 ;
}
void NullFXP::slot_cancel_connect()
{
    qDebug() <<__FUNCTION__<<": "<<__LINE__<<":"<< __FILE__;
    this->remote_conn_thread->set_user_canceled();
}

void NullFXP::slot_new_upload_requested ( QStringList local_file_names )
{
	qDebug() <<__FUNCTION__<<": "<<__LINE__<<":"<< __FILE__;
	//QString remote_file_name ;
	//QStringList remote_file_names ;

    RemoteView * remote_view = this->get_top_most_remote_view() ;
    if( remote_view == 0 )
    {
        qDebug()<<" may be not connected ";
        return ;
    }
    remote_view->slot_new_upload_requested( local_file_names ) ;

}


void NullFXP::slot_show_transfer_queue ( bool show )
{
	qDebug() <<__FUNCTION__<<": "<<__LINE__<<":"<< __FILE__;
	transfer_queue_list_view->setVisible ( show );
}
void NullFXP::slot_show_fxp_command_log ( bool show )
{
	qDebug() <<__FUNCTION__<<": "<<__LINE__<<":"<< __FILE__;
    RemoteView * remote_view = this->get_top_most_remote_view() ;
    if( remote_view !=0 )
            remote_view->slot_show_fxp_command_log ( show )  ;
}

void NullFXP::slot_cascade_sub_windows()
{
	qDebug() <<__FUNCTION__<<": "<<__LINE__<<":"<< __FILE__;
	this->mdiArea->cascadeSubWindows();
}
void NullFXP::slot_tile_sub_windows()
{
	qDebug() <<__FUNCTION__<<": "<<__LINE__<<":"<< __FILE__;
    //让本地视图总是显示在左侧,并不改为原来的窗口顺序
    QMdiSubWindow * curr_active_sub_window = this->mdiArea->activeSubWindow ();
    this->mdiArea->setActiveSubWindow ( this->mdiArea->subWindowList(QMdiArea::CreationOrder) .at ( 0 ) );
	this->mdiArea->tileSubWindows();
    this->mdiArea->setActiveSubWindow ( curr_active_sub_window );
}

void NullFXP::slot_show_local_view()
{
	qDebug() <<__FUNCTION__<<": "<<__LINE__<<":"<< __FILE__;
	if ( !this->localView->isVisible() )
	{
		this->localView->setVisible ( true );
	}
    this->mdiArea->setActiveSubWindow ( this->mdiArea->subWindowList(QMdiArea::CreationOrder) .at ( 0 ) );
}

void NullFXP::slot_show_remote_view()
{
	qDebug() <<__FUNCTION__<<": "<<__LINE__<<":"<< __FILE__;
    
    //将最上面一个RemoteView提升起来，条件是这个RemoteView现在还不是最上面的一个子窗口。

    QList<QMdiSubWindow *> sub_window_list = this->mdiArea->subWindowList(QMdiArea::StackingOrder);
    int sub_wnd_count = sub_window_list.count() ;
    
    for( sub_wnd_count = sub_wnd_count -1 ;  sub_wnd_count >= 0  ; sub_wnd_count -- )
    {
        if( sub_window_list.at( sub_wnd_count )->widget() != this->localView 
            && sub_window_list.at( sub_wnd_count )->widget()->objectName()=="rv" )
        {
            this->mdiArea->setActiveSubWindow ( sub_window_list.at( sub_wnd_count ) );
            break ;
        }
    }
}

RemoteView * NullFXP::get_top_most_remote_view () 
{
    RemoteView * remote_view = 0 ;
    QList<QMdiSubWindow *> sub_window_list = this->mdiArea->subWindowList(QMdiArea::StackingOrder);
    int sub_wnd_count = sub_window_list.count() ;
    
    for( sub_wnd_count = sub_wnd_count -1 ;  sub_wnd_count >= 0  ; sub_wnd_count -- )
    {
        if( sub_window_list.at( sub_wnd_count )->widget() != this->localView 
            && sub_window_list.at( sub_wnd_count )->widget()->objectName()=="rv" )
        {
            remote_view = static_cast<RemoteView*>( sub_window_list.at( sub_wnd_count )->widget() );
            break ;
        }
    }
    return remote_view ;
}








