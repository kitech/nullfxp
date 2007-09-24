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
#ifndef REMOTEVIEW_H
#define REMOTEVIEW_H

#include <QWidget>
#include <QtCore>
#include <QVector>
#include <QPair>

#include "ui_remoteview.h"

// #include "sftp-client.h"
// #include "sftp-common.h"

#include "remotedirmodel.h"

#include "libssh2.h"
#include "libssh2_sftp.h"

/**
	@author liuguangzhao <gzl@localhost>
*/
class RemoteView : public QWidget
{
    Q_OBJECT
public:
    RemoteView(QWidget *parent = 0);

    ~RemoteView();
    QString get_selected_directory();
    //
    void set_ssh2_handler( void * ssh2_sess , void * ssh2_sftp, int ssh2_sock );
    void set_user_home_path(std::string user_home_path);
    
    bool is_in_remote_dir_retrive_loop() 
        { return this->in_remote_dir_retrive_loop ; }
    
    void update_layout();
    
    private:
        
        QStatusBar * status_bar ;
        
        Ui::remoteview remoteview;
        RemoteDirModel * remote_dir_model ;
        
        QMenu * dir_tree_context_menu ;
        
        void init_popup_context_menu() ;
        
        std::string user_home_path ;    
        
        QCursor orginal_cursor ;
        bool    in_remote_dir_retrive_loop ;
        
        //
        int     ssh2_sock ;
        LIBSSH2_SESSION * ssh2_sess ;
        LIBSSH2_CHANNEL *ssh2_channel;
        LIBSSH2_SFTP * ssh2_sftp ;
                
    public slots:
        void i_init_dir_view(/*struct sftp_conn * conn*/);
        void slot_disconnect_from_remote_host();
        
        void slot_dir_tree_customContextMenuRequested ( const QPoint & pos );
        void slot_new_transfer();
        void slot_new_transfer_requested(QStringList local_file_names,                                    QStringList remote_file_names);
        
        //////////ui
        void slot_show_fxp_command_log(bool show);
        
        void slot_custom_ui_area();
        
        void slot_enter_remote_dir_retrive_loop();
        void slot_leave_remote_dir_retrive_loop();
        
    signals:
        //void new_transfer_requested( QString file_name,QString file_type ) ;
        void new_transfer_requested(QStringList remote_file_names);
        //first is file_name , second is file_type 
        //可以多选的时候使用。
        //void new_transfer_requested( QVector<QPair<QString ,QString > > file_lists ) ;
        //void new_transfer_requested(QString local_file_name,QString local_file_type,
        //                            QString remote_file_name,QString remote_file_type );
        void new_transfer_requested(QStringList local_file_names,QStringList remote_file_names);
        
    private slots:
        void slot_dir_item_clicked(const QModelIndex & index);

        void slot_refresh_directory_tree();
        void slot_mkdir();
        void slot_rmdir();        
        void slot_rename();
        
        void rm_file_or_directory_recursively();
            
    protected:
        virtual void closeEvent ( QCloseEvent * event ) ;
};

#endif
