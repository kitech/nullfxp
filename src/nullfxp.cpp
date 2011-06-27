// nullfxp.cpp --- 
// 
// Author: liuguangzhao
// Copyright (C) 2007-2010 liuguangzhao@users.sf.net
// URL: http://www.qtchina.net http://nullget.sourceforge.net
// Created: 2008-07-22 21:57:47 +0800
// Version: $Id$
// 

#include <cstdlib>
#include <cstdio>
#include <sys/stat.h>
#include <sys/types.h>
#include <cerrno>

#ifdef WIN32
#include <winsock2.h>
#else
#include <sys/socket.h>
#endif

#include <QtNetwork>
#include <QCoreApplication>

#include "utils.h"
#include "ui_nullfxp.h"
#include "nullfxp.h"
#include "aboutnullfxp.h" 
#include "globaloptionsdialog.h"
#include "localview.h"
#include "progressdialog.h"
#include "connectingstatusdialog.h"
#include "quickconnectinfodialog.h"
#include "sessiondialog.h"
#include "connection.h"
#include "connector.h"
#include "ftpview.h"
#include "sftpview.h"
#include "updatedialog.h"
#include "taskqueueview.h"
#include "globaloption.h"

extern GlobalOption *gOpt;

//////////////////////////
NullFXP::NullFXP(QWidget *parent, Qt::WindowFlags flags)
: QMainWindow(parent, flags)
    , mUIMain(new Ui::MainWindow())
{
    this->mUIMain->setupUi(this);
    this->setWindowIcon(QIcon(":/icons/nullget-1.png")); 

    //////////////////////////
    central_splitter_widget = new QSplitter(Qt::Vertical);
    this->fcd = 0;
    this->connector = NULL;

    //
    this->mdiArea = new QMdiArea;
    this->mdiArea->setWindowIcon(QIcon(":/icons/nullget-2.png")); 
    QObject::connect(this->mUIMain->actionTransfer_queue, SIGNAL(triggered(bool)),
                     this, SLOT(slot_show_transfer_queue(bool)));
    QObject::connect(this->mUIMain->actionShow_log,SIGNAL(triggered(bool)),
                     this, SLOT(slot_show_fxp_command_log(bool)));
  
    QObject::connect(this->mUIMain->actionCascade_window, SIGNAL(triggered(bool)),
                     this, SLOT(slot_cascade_sub_windows(bool)));
    QObject::connect(this->mUIMain->actionTile_window, SIGNAL(triggered(bool)),
                     this, SLOT(slot_tile_sub_windows(bool)));
  
    this->task_queue_view = new TaskQueueView();
    this->central_splitter_widget->addWidget(mdiArea);
    this->central_splitter_widget->addWidget(this->task_queue_view);

    QSizePolicy sp;
    sp.setVerticalPolicy(QSizePolicy::Ignored);
    this->task_queue_view->setSizePolicy(sp);
    // this->task_queue_view->setVisible(false);
  
    this->setCentralWidget(this->central_splitter_widget);

    windowMapper = new QSignalMapper(this);

    ///////////////////////
    QObject::connect(this->mUIMain->actionConnect, SIGNAL(triggered()),
                       this, SLOT(connect_to_remote_host()));
    QObject::connect(this->mUIMain->actionDisconnect, SIGNAL(triggered()),
                     this, SLOT(slot_disconnect_from_remote_host()));
    QObject::connect(this->mUIMain->actionSession, SIGNAL(triggered()),
                     this, SLOT(slot_show_session_dialog()));
    QObject::connect(this->mUIMain->action_Global_Options, SIGNAL(triggered()),
                     this, SLOT(slot_show_option_dialog()));

    localView = new LocalView(this->mdiArea);

    mdiArea->addSubWindow(localView);

    QObject::connect(localView, SIGNAL(new_upload_requested(TaskPackage)),
                     this, SLOT(slot_new_upload_requested(TaskPackage)));

    //
    QObject::connect(this->mUIMain->action_Local_Window, SIGNAL(triggered(bool)),
                       this,SLOT(slot_show_local_view(bool)));
    QObject::connect(this->mUIMain->action_Remote_Window, SIGNAL(triggered(bool)),
                     this, SLOT(slot_show_remote_view(bool)));

    //////////////
    QObject::connect(this->mUIMain->actionCheck_for_update, SIGNAL(triggered()),
                     this, SLOT(slot_check_for_updates()));
    this->about_nullfxp_dialog = NULL;
    QObject::connect(this->mUIMain->actionAbout_NullFXP, SIGNAL(triggered()),
                     this, SLOT(slot_about_nullfxp()));
    QObject::connect(this->mUIMain->actionAbout_Qt, SIGNAL(triggered()),
                     qApp, SLOT(aboutQt()));

    //tool menu
    QObject::connect(this->mUIMain->action_Forward_connect, SIGNAL(triggered(bool)),
                     this, SLOT(slot_forward_connect(bool)));
    QObject::connect(this->mUIMain->action_Synchronize_file, SIGNAL(triggered()),
                     this, SLOT(slot_synchronize_file()));
    
    this->central_splitter_widget->setStretchFactor(0, 3);
    this->central_splitter_widget->setStretchFactor(1, 1);

    // force show event
    this->show();
    // 根据当前屏幕大小调整界面的大小。
    this->desktop = QApplication::desktop();
    //qDebug()<<dw->screenGeometry();
    // this->resize(dw->screenGeometry().width()*5/6, dw->screenGeometry().height()*5/6);
    // delete dw;
    // #bug245
    int screenCount = 1;
#if QT_VERSION >= 0x040600
    if (strcmp(qVersion(), "4.6.0") >= 0) {
        screenCount = this->desktop->screenCount();
    }
#endif
    qDebug()<<"Desktop screen count:"<<screenCount;
    if (screenCount == 1) {
        this->resize(this->desktop->screenGeometry().width()*5/6, 
                     this->desktop->screenGeometry().height()*5/6);
    } else {
        // this->resize(this->desktop->screenGeometry(this->desktop->primaryScreen()).width()*5/6, 
        //              this->desktop->screenGeometry(this->desktop->primaryScreen()).height()*5/6);
        this->resize(this->desktop->screenGeometry().width()*6/10, 
                     this->desktop->screenGeometry().height()*7/10);
    }

    // 调整本地目录树窗口的大小
    //QList<QMdiSubWindow *> mdiSubWindow = mdiArea->subWindowList();
    //qDebug()<<"mdi sub window count :"<< mdiSubWindow.count();
    QMdiSubWindow *local_sub_win = mdiArea->subWindowList().at(0);
    local_sub_win->setGeometry(local_sub_win->x(), local_sub_win->y(), mdiArea->width()/2, mdiArea->height()*19/19);

    QActionGroup *flv_ag = new QActionGroup(this);
    flv_ag->addAction(this->mUIMain->actionLar_ge_Icons);
    flv_ag->addAction(this->mUIMain->actionS_mall_Icons);
    flv_ag->addAction(this->mUIMain->action_List);
    flv_ag->addAction(this->mUIMain->action_Details);
    QObject::connect(flv_ag, SIGNAL(triggered(QAction*)),
                     this, SLOT(slot_set_file_list_mode(QAction*)));

    BaseStorage *storage = BaseStorage::instance();
    int host_count = storage->hostCount();
    if (host_count > 0) {
        QTimer::singleShot(20, this, SLOT(slot_show_session_dialog()));
    } else {
        QTimer::singleShot(20, this, SLOT(connect_to_remote_host()));
    }

    // QTimer::singleShot(10, this, SLOT(slot_set_mdi_area_background()));
    //////////////////////
    //this->mUIMain->action_Forward_connect->setVisible(false);
    //this->mUIMain->action_Synchronize_file->setVisible(false);
}

NullFXP::~NullFXP()
{
}
void NullFXP::slot_about_nullfxp()
{
    if (this->about_nullfxp_dialog == NULL) {
        this->about_nullfxp_dialog = new AboutNullFXP();
    }
    this->about_nullfxp_dialog->setVisible(true);
}

void NullFXP::connect_to_remote_host()
{
    qDebug()<<__FUNCTION__<<": "<<__LINE__<<":"<< __FILE__;

    QString protocol;
    QString username;
    QString password;
    QString remoteaddr;
    short   port;
    QString pubkey = QString::null;
    QMap<QString,QString> host, old_host;

    this->quick_connect_info_dailog = new QuickConnectInfoDialog(this);
    if (this->quick_connect_info_dailog->exec() == QDialog::Accepted) {
        protocol = this->quick_connect_info_dailog->get_protocol();
        username = this->quick_connect_info_dailog->get_user_name();
        password = this->quick_connect_info_dailog->get_password();
        remoteaddr = this->quick_connect_info_dailog->get_host_name();
        port = this->quick_connect_info_dailog->get_port();
        pubkey = this->quick_connect_info_dailog->get_pubkey();

        delete this->quick_connect_info_dailog;
        this->quick_connect_info_dailog = 0;

        host["show_name"] = remoteaddr;
        host["protocol"] = protocol;
        host["host_name"] = remoteaddr;
        host["user_name"] = username;
        host["password"] = password;
        host["port"] = QString("%1").arg(port);
        (pubkey == QString::null) ? (pubkey = QString::null) : (host["pubkey"] = pubkey);
        
        BaseStorage *storage = BaseStorage::instance();
        if (storage->containsHost(remoteaddr)) {
            storage->updateHost(host);
        } else {
            storage->addHost(host);
        }
        host.clear();
        host = storage->getHost(remoteaddr);
        //delete storage;
        this->connect_to_remote_host(host);
    } else {
        qDebug()<<"user canceled ...";
    }

}

void NullFXP::connect_to_remote_host(QMap<QString, QString> host)
{
    QString protocol;
    protocol = host["protocol"];
    if (protocol == "FTPS") {
        // q_debug()<<"Not impled yet";
        this->connect_to_remote_host2(host);
    } else if (protocol == "FTPES") {
        // q_debug()<<"Not impled yet.";
        this->connect_to_remote_host2(host);
    } else if (protocol == "FTP") {
        this->connect_to_remote_host2(host);
    } else if (protocol == "SFTP") {
        // this->connect_to_remote_sftp_host(host);
        this->connect_to_remote_host2(host);
    } else {
        q_debug()<<"Unknown host protocol:"<<protocol;
    }
}

void NullFXP::connect_to_remote_host2(QMap<QString, QString> host)
{
    q_debug()<<"";
    QString username;
    QString hostname;
    QString port;

    username = host["user_name"];
    hostname = host["host_name"];
    port = host["port"];

    this->connect_status_dailog = new ConnectingStatusDialog(username, hostname, port, this, Qt::Dialog);
    QObject::connect(this->connect_status_dailog, SIGNAL(cancel_connect()),
                     this, SLOT(slot_cancel_connect()));

    this->connector = new Connector();
    this->connector->setHostInfo(host);
    QObject::connect(this->connector, SIGNAL(connect_finished(int, Connection *)),
                     this, SLOT(slot_connect_remote_host_finished (int, Connection *)));

    QObject::connect(this->connector, SIGNAL(connect_state_changed(QString)),
                     this->connect_status_dailog, SLOT(slot_connect_state_changed(QString)));

    this->connector->start();
    this->connect_status_dailog->exec();
}

void NullFXP::slot_disconnect_from_remote_host()
{
    qDebug()<<__FUNCTION__<<": "<<__LINE__<<":"<< __FILE__;

    RemoteView *remote_view = this->get_top_most_remote_view();
    
    if (remote_view == 0) {
        //do nothing
    } else {
        qDebug()<<"disconnect: "<<remote_view->windowTitle();
        QList<QMdiSubWindow *> sub_window_list = this->mdiArea->subWindowList(QMdiArea::StackingOrder);
        int sub_wnd_count = sub_window_list.count() ;
    
        for (sub_wnd_count = sub_wnd_count -1 ; sub_wnd_count >= 0 ; sub_wnd_count --) {
            if (sub_window_list.at(sub_wnd_count)->widget() != this->localView 
                && sub_window_list.at(sub_wnd_count)->widget()->objectName() == "NetDirView") {
                sub_window_list.at(sub_wnd_count)->close();
                break ;
            }
        }
    }
}

void NullFXP::slot_show_session_dialog()
{
    SessionDialog *sess_dlg = new SessionDialog(this);
    QObject::connect(sess_dlg, SIGNAL(quick_connect()), this, SLOT(connect_to_remote_host()));
    if (sess_dlg->exec() == QDialog::Accepted) {
        QMap<QString,QString> host;
        host = sess_dlg->get_host_map();
        this->connect_to_remote_host(host);
    }
    delete sess_dlg;
}

void NullFXP::slot_show_option_dialog()
{
    GlobalOptionsDialog *dlg = new GlobalOptionsDialog();
    dlg->exec();
}

void NullFXP::slot_connect_remote_host_finished(int status, Connection *conn)
{
    q_debug()<<status<<conn;
    Connector *connector = static_cast<Connector*>(sender());
    
    qDebug() <<__FUNCTION__<<": "<<__LINE__<<":"<< __FILE__;
    if (status == Connection::CONN_OK ) {
        RemoteView *view = NULL;
        switch (conn->protocolType()) {
        case Connection::PROTO_FTPS:
            view = new FTPView(this->mdiArea, this->localView);
            break;
        case Connection::PROTO_FTPES:
            view = new FTPView(this->mdiArea, this->localView);
            break;
        case Connection::PROTO_FTP:
            view = new FTPView(this->mdiArea, this->localView);
            break;
        case Connection::PROTO_SFTP:
            view = new SFTPView(this->mdiArea, this->localView);
            break;
        default:
            q_debug()<<"Unsupported protocol:"<<conn->protocolType();
            assert(1 == 2);
            break;
        } 
        
        mdiArea->addSubWindow(view);
        QMdiSubWindow *local_sub_win = mdiArea->subWindowList(QMdiArea::CreationOrder)
            .at(mdiArea->subWindowList(QMdiArea::CreationOrder).count() - 1);
        
        view->slot_custom_ui_area();
        local_sub_win->show();
        // resize view window size
        //QList<QMdiSubWindow *> mdiSubWindow = mdiArea->subWindowList();
        //qDebug()<<" mdi sub window count :"<< mdiSubWindow.count();        
        //local_sub_win->setGeometry( local_sub_win->x(),local_sub_win->y(), mdiArea->width()/2,  mdiArea->height()*18/19 );
        local_sub_win->resize(mdiArea->width()/2, mdiArea->height()*18/19);
        
        view->set_user_home_path(conn->userHomePath());
        view->setConnection(conn);

        // 初始化远程目录树        
        view->i_init_dir_view();
    } else if (status == Connection::CONN_CANCEL) {   //use canceled connection
        qDebug()<<"user canceled connecting";
        if (conn != NULL) {
            delete conn;
        }
    } else {
        //assert ( 1==2 );
        //this->connect_status_dailog->setVisible(false);
        this->connect_status_dailog->stop_progress_bar();
    
        QString emsg = connector->get_status_desc(status);
        QString errstr = connector->errorString();
        QMessageBox::critical(this, tr("Connect Error:"), errstr);
        if (conn != NULL) {
            delete conn;
        }
    }

    this->connect_status_dailog->accept();
    delete this->connect_status_dailog;
    this->connect_status_dailog = 0;
    this->connector->deleteLater();
    this->connector = 0;    
}

void NullFXP::slot_cancel_connect()
{
    qDebug() <<__FUNCTION__<<": "<<__LINE__<<":"<< __FILE__;
    this->connector->setUserCanceled();
}

void NullFXP::slot_new_upload_requested(TaskPackage local_pkg)
{
    qDebug() <<__FUNCTION__<<": "<<__LINE__<<":"<< __FILE__;
    // TODO, maybe has problem when use drag/drop to the window,
    // but the window is not the toppest window.
    RemoteView *remote_view = this->get_top_most_remote_view();
    if (remote_view == 0) {
        qDebug()<<"may be not connected";
        return;
    }
    remote_view->slot_new_upload_requested(local_pkg);
}

void NullFXP::slot_show_transfer_queue(bool show)
{
    qDebug()<<__FUNCTION__<<": "<<__LINE__<<":"<< __FILE__;
    task_queue_view->setVisible(show);
}

void NullFXP::slot_show_fxp_command_log(bool show)
{
    qDebug()<<__FUNCTION__<<": "<<__LINE__<<":"<< __FILE__;
    RemoteView *remote_view = this->get_top_most_remote_view();
    if (remote_view != 0) {
        remote_view->slot_show_fxp_command_log(show);
    }
}

void NullFXP::slot_cascade_sub_windows(bool triggered)
{
    qDebug() <<__FUNCTION__<<": "<<__LINE__<<":"<< __FILE__;    
    if (triggered) {
        this->mdiArea->cascadeSubWindows();
    }

    this->mUIMain->actionTile_window->setChecked(!triggered);
}
void NullFXP::slot_tile_sub_windows(bool triggered)
{
    qDebug() <<__FUNCTION__<<": "<<__LINE__<<":"<< __FILE__;
    if (triggered) {
        // 让本地视图总是显示在左侧,并不改为原来的窗口顺序
        QMdiSubWindow *curr_active_sub_window = this->mdiArea->activeSubWindow();
        this->mdiArea->setActiveSubWindow(this->mdiArea->subWindowList(QMdiArea::CreationOrder).at(0));
        this->mdiArea->tileSubWindows();
        this->mdiArea->setActiveSubWindow(curr_active_sub_window);
    }
    this->mUIMain->actionCascade_window->setChecked(!triggered);
}

void NullFXP::slot_show_local_view(bool triggered)
{
    qDebug() <<__FUNCTION__<<": "<<__LINE__<<":"<< __FILE__;
    if (triggered) {
        if (!this->localView->isVisible()) {
            this->localView->setVisible(true);
        }
        this->mdiArea->setActiveSubWindow(this->mdiArea->subWindowList(QMdiArea::CreationOrder).at(0));
    }
    this->mUIMain->action_Remote_Window->setChecked(!triggered);
}

void NullFXP::slot_show_remote_view(bool triggered)
{
    qDebug() <<__FUNCTION__<<": "<<__LINE__<<":"<< __FILE__;
    
    if (triggered) {
        // set the toppest RemoteView window, if not
        QList<QMdiSubWindow *> sub_window_list = this->mdiArea->subWindowList(QMdiArea::StackingOrder);
        int sub_wnd_count = sub_window_list.count();
    
        for (sub_wnd_count = sub_wnd_count -1 ; sub_wnd_count >= 0 ; sub_wnd_count --) {
            if (sub_window_list.at( sub_wnd_count )->widget() != this->localView 
                && sub_window_list.at(sub_wnd_count)->widget()->objectName() == "NetDirView") {
                this->mdiArea->setActiveSubWindow(sub_window_list.at(sub_wnd_count));
                break;
            }
        }
    }
    this->mUIMain->action_Local_Window->setChecked(!triggered);
}

RemoteView *NullFXP::get_top_most_remote_view() 
{
    RemoteView *remote_view = 0;
    QList<QMdiSubWindow *> sub_window_list = this->mdiArea->subWindowList(QMdiArea::StackingOrder);
    int sub_wnd_count = sub_window_list.count() ;
    
    for (sub_wnd_count = sub_wnd_count -1 ; sub_wnd_count >= 0 ; sub_wnd_count --) {
        if (sub_window_list.at( sub_wnd_count )->widget() != this->localView 
            && sub_window_list.at(sub_wnd_count)->widget()->objectName() == "NetDirView") {
            remote_view = static_cast<RemoteView*>(sub_window_list.at(sub_wnd_count)->widget());
            break ;
        }
    }
    return remote_view;
}

void NullFXP::slot_set_file_list_mode(QAction *action)
{
    RemoteView *remote_view = 0;
    QList<QMdiSubWindow *> sub_window_list = this->mdiArea->subWindowList(QMdiArea::StackingOrder);
    int sub_wnd_count = sub_window_list.count() ;

    if (action == this->mUIMain->actionLar_ge_Icons) {
        gOpt->file_list_view_mode = GlobalOption::FLV_LARGE_ICON;
    } else if (action == this->mUIMain->actionS_mall_Icons) {
        gOpt->file_list_view_mode = GlobalOption::FLV_SMALL_ICON;
    } else if (action == this->mUIMain->action_List) {
        gOpt->file_list_view_mode = GlobalOption::FLV_LIST;
    } else if (action == this->mUIMain->action_Details) {
        gOpt->file_list_view_mode = GlobalOption::FLV_DETAIL;
    } else {
        Q_ASSERT(1 == 2);
    }
    
    for (sub_wnd_count = sub_wnd_count -1 ; sub_wnd_count >= 0 ; sub_wnd_count --) {
        if (sub_window_list.at(sub_wnd_count)->widget() == this->localView) {
            this->localView->setFileListViewMode(gOpt->file_list_view_mode);
        } else if (sub_window_list.at(sub_wnd_count)->widget() != this->localView
                   && sub_window_list.at(sub_wnd_count)->widget()->objectName() == "NetDirView") {
            remote_view = static_cast<RemoteView*>(sub_window_list.at(sub_wnd_count)->widget());
            remote_view->setFileListViewMode(gOpt->file_list_view_mode);
        } else {
            // maybe prograss window
        }
    }

}

void NullFXP::slot_check_for_updates()
{
    // q_debug()<<""<<"curr version:"<<NULLFXP_VERSION_STR;
    UpdateDialog *dlg = new UpdateDialog();
    int ret = dlg->exec();
    if (ret == QDialog::Accepted) {
    } else {
    }
    delete dlg;
}
