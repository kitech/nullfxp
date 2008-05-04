/***************************************************************************
 *   Copyright (C) 2007-2008 by liuguangzhao   *
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
#ifndef REMOTEVIEW_H
#define REMOTEVIEW_H

#include <QWidget>
#include <QtCore>
#include <QVector>
#include <QPair>

class ProgressDialog;
class RemoteDirSortFilterModel ;
class RemoteDirSortFilterModelEX ;

#include "ui_remoteview.h"

#include "remotedirmodel.h"

#include "libssh2.h"
#include "libssh2_sftp.h"

class LocalView ;
class EncryptionDetailFocusLabel;

/**
	@author liuguangzhao <liuguangzhao@users.sourceforge.net >
*/
class RemoteView : public QWidget
{
    Q_OBJECT
public:
    RemoteView( QMdiArea * main_mdi_area , LocalView * local_view , QWidget *parent = 0);

    ~RemoteView();
    QString get_selected_directory();
    //
    void set_host_info ( QString host_name ,QString user_name, QString password, short port );
    void set_ssh2_handler( void * ssh2_sess /*, void * ssh2_sftp*/, int ssh2_sock );
    void set_user_home_path(std::string user_home_path);
    
    bool is_in_remote_dir_retrive_loop() 
        { return this->in_remote_dir_retrive_loop ; }
    
    void update_layout();
    
//     LIBSSH2_SESSION * get_ssh2_sess();
//     LIBSSH2_SFTP * get_ssh2_sftp ();
//     int get_ssh2_sock ( );
    
    private:
        LocalView  * local_view ;
        QMdiArea   * main_mdi_area ;
        QStatusBar * status_bar ;
        
        Ui::remoteview remoteview;
        RemoteDirModel * remote_dir_model ;
        int   table_row_height ;
        RemoteDirSortFilterModel * remote_dir_sort_filter_model ;
        RemoteDirSortFilterModelEX * remote_dir_sort_filter_model_ex ;
        QAbstractItemView * curr_item_view ;    //
        
        QMenu * dir_tree_context_menu ;
        
        void init_popup_context_menu() ;
        
        std::string user_home_path ;    
        
        QCursor orginal_cursor ;
        bool    in_remote_dir_retrive_loop ;
        ProgressDialog * own_progress_dialog ;
        
        //
        int     ssh2_sock ;
        LIBSSH2_SESSION * ssh2_sess ;
        LIBSSH2_CHANNEL *ssh2_channel;
        LIBSSH2_SFTP * ssh2_sftp ;
        
        QString host_name ;
        QString user_name ;
        QString password ;
        short  port;
        
        //menu item
        QAction * attr_action;
	EncryptionDetailFocusLabel * enc_label;
        
    public slots:
        void i_init_dir_view( );
        void slot_disconnect_from_remote_host();
        
        void slot_dir_tree_customContextMenuRequested ( const QPoint & pos );
        void slot_new_transfer();
        //void slot_new_transfer_requested(QStringList local_file_names,                                    QStringList remote_file_names);
        void slot_new_upload_requested(QStringList local_file_names,                                    QStringList remote_file_names);
        void slot_new_upload_requested( QStringList local_file_names ) ;
        void slot_new_download_requested(QStringList local_file_names,                                    QStringList remote_file_names);
        void slot_new_download_requested( QStringList remote_file_names ) ;
        
        //////////ui
        void slot_show_fxp_command_log(bool show);
        
        void slot_custom_ui_area();
        
        void slot_enter_remote_dir_retrive_loop();
        void slot_leave_remote_dir_retrive_loop();
        
        void slot_transfer_finished( int status ) ;
        
        //view drag
        void slot_drag_ready();
        
        //
        bool slot_drop_mime_data(const QMimeData *data, Qt::DropAction action,
                                 int row, int column, const QModelIndex &parent ) ;
        void slot_show_hidden(bool show);
	
	void slot_ssh_server_info();
        
    signals:
        //void new_transfer_requested( QString file_name,QString file_type ) ;
        //void new_transfer_requested(QStringList remote_file_names);
        //first is file_name , second is file_type 
        //可以多选的时候使用。
        //void new_transfer_requested( QVector<QPair<QString ,QString > > file_lists ) ;
        //void new_transfer_requested(QString local_file_name,QString local_file_type,
        //                            QString remote_file_name,QString remote_file_type );
        void new_transfer_requested(QStringList local_file_names,QStringList remote_file_names);
        
    private slots:
        //void slot_dir_item_clicked(const QModelIndex & index);

        void slot_refresh_directory_tree();
        void slot_show_properties();
        void slot_mkdir();
        void slot_rmdir();        
        void slot_rename();
        
        void rm_file_or_directory_recursively();

        void slot_dir_tree_item_clicked( const QModelIndex & index);
        void slot_dir_file_view_double_clicked( const QModelIndex & index );

	void encryption_focus_label_double_clicked();
        
    protected:
        virtual void closeEvent ( QCloseEvent * event ) ;
};

#endif
