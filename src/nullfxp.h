/* nullfxp.h --- 
 * 
 * Author: liuguangzhao
 * Copyright (C) 2007-2010 liuguangzhao@users.sf.net
 * URL: http://www.qtchina.net http://nullget.sourceforge.net
 * Created: 2008-05-06 22:02:06 +0800
 * Last-Updated: 2009-05-07 23:16:48 +0800
 * Version: $Id$
 */

#ifndef NULLFXP_H
#define NULLFXP_H

#include <QObject>
#include <QtCore>
#include <QtGui>
#include <QMainWindow>
#include <QMessageBox>
#include <QTreeWidget>
#include <QStandardItemModel>
#include <QItemSelectionModel>
#include <QItemSelection>
#include <QTime>
#include <QDate>
#include <QDateTime>
#include <QImage>
#include <QPixmap>
#include <QTimer>
#include <QSystemTrayIcon>
#include <QPalette>

 class QAction;
 class QMenu;
 class QMdiArea;
 class QMdiSubWindow;
 class MdiChild;
 class QSignalMapper;
 
#include "ui_nullfxp.h"

// #include "localview.h"
// #include "remoteview.h"
// #include "remotehostconnectingstatusdialog.h"
// #include "remotehostquickconnectinfodialog.h"
// #include "remotehostconnectthread.h"
class LocalView;
         class RemoteView;
         class RemoteHostConnectingStatusDialog;
         class RemoteHostQuickConnectInfoDialog;
         class RemoteHostConnectThread ;
         class ForwardConnectDaemon;

#include "aboutnullfxp.h"
                 
/**
	@author liuguangzhao <gzl@localhost>
*/
class NullFXP : public QMainWindow
{
    Q_OBJECT
                       
public:
    NullFXP(QWidget * parent = 0, Qt::WindowFlags flags = 0);

    ~NullFXP();
    
    public slots:
        void slot_about_nullfxp();
        //void test();
        //void do_init();
        //void do_ls () ;
        //void local_init_dir_view();
        
        void connect_to_remote_host() ;
        void connect_to_remote_host(QMap<QString,QString> host) ;
        void slot_disconnect_from_remote_host();
        void slot_cancel_connect();
        void slot_connect_remote_host_finished(int status,void * ssh2_sess , int ssh2_sock /*, void * ssh2_sftp */);
        
        void slot_new_upload_requested(QStringList local_file_names);        

        //void slot_new_upload_requested(QStringList local_file_names,QStringList remote_file_names );
        //void slot_new_download_requested(QStringList remote_file_names );

        //void slot_new_download_requested(QStringList local_file_names, QStringList remote_file_names );
        
        //void slot_transfer_finished(int status );
        
        void slot_show_local_view(bool);
        void slot_show_remote_view(bool);
        void slot_cascade_sub_windows(bool triggered);
        void slot_tile_sub_windows(bool triggered);
        void slot_show_transfer_queue(bool show);
        void slot_show_fxp_command_log(bool show);
        
    private slots:
        void slot_forward_connect(bool show);
        void slot_synchronize_file();
	void slot_show_session_dialog();

    private:
        MdiChild *activeMdiChild();        

        QMdiArea *mdiArea;
        QSignalMapper *windowMapper;                
        //QList<QMdiSubWindow *>  sub_windows ;
        RemoteView * get_top_most_remote_view () ;
        
    private:
        
        QSplitter * central_splitter_widget ;
        QListView * transfer_queue_list_view;
                
        LocalView * localView;
        //RemoteView * remoteView ;
        
        Ui::MainWindow mUIMain;
        AboutNullFXP  * about_nullfxp_dialog;
        
        RemoteHostConnectingStatusDialog * connect_status_dailog ;
        RemoteHostQuickConnectInfoDialog * quick_connect_info_dailog ;
        RemoteHostConnectThread * remote_conn_thread ;

        ForwardConnectDaemon * fcd;
    private:
        //connection can not store here
        //void * ssh2_sess ;
        //void * ssh2_sftp ;
        //int ssh2_sock ;
};

#endif
