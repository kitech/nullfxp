/***************************************************************************
 *   Copyright (C) 2007 by liuguangzhao   *
 *   gzl@localhost   *
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
#include "sftp-client.h"

#include "localview.h"
#include "remoteview.h"
#include "remotehostconnectingstatusdialog.h"
#include "remotehostquickconnectinfodialog.h"
#include "remotehostconnectthread.h"
                 
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
        void slot_disconnect_from_remote_host();
        
        void slot_connect_remote_host_finished(int status , struct sftp_conn * conn );
        
        void slot_new_upload_requested(QString local_file_name,QString local_file_type );
        void slot_new_upload_requested(QString local_file_name,QString local_file_type ,
                                      QString remote_file_name,QString remote_file_type );
        void slot_new_download_requested(QString remote_file_name,QString remote_file_type );
        void slot_new_download_requested(QString local_file_name,QString local_file_type,
                                         QString remote_file_name,QString remote_file_name);
        
        
        void slot_transfer_finished(int status );
        
        void slot_show_local_view();
        void slot_show_remote_view();
        void slot_cascade_sub_windows();
        void slot_tile_sub_windows();
        void slot_show_transfer_queue(bool show);
        void slot_show_fxp_command_log(bool show);
        
    private:
        MdiChild *activeMdiChild();        

        QMdiArea *mdiArea;
        QSignalMapper *windowMapper;                
        
        
    private:
        
        QSplitter * central_splitter_widget ;
        QListView * transfer_queue_list_view;
                
        LocalView * localView;
        RemoteView * remoteView ;
        
        Ui::MainWindow mUIMain;
        AboutNullFXP  * about_nullfxp_dialog;
        
        RemoteHostConnectingStatusDialog * connect_status_dailog ;
        RemoteHostQuickConnectInfoDialog * quick_connect_info_dailog ;
        RemoteHostConnectThread * remote_conn_thread ;
        /*
        struct sftp_conn {
            int fd_in;
            int fd_out;
            u_int transfer_buflen;
            u_int num_requests;
            u_int version;
            u_int msg_id;
        };
        */
        struct sftp_conn * sftp_connection ;
};

#endif
