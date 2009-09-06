/* ftpview.h --- 
 * 
 * Author: liuguangzhao
 * Copyright (C) 2007-2010 liuguangzhao@users.sf.net
 * URL: http://www.qtchina.net http://nullget.sourceforge.net
 * Created: 2008-05-31 22:09:15 +0800
 * Version: $Id$
 */


#ifndef FTPVIEW_H
#define FTPVIEW_H

#include <QWidget>
#include <QtCore>
#include <QVector>
#include <QPair>

#include "libssh2.h"
#include "libssh2_sftp.h"

#include "taskpackage.h"
#include "remotedirmodel.h"

#include "ui_remoteview.h"

#include "remoteview.h"

class ProgressDialog;
class RemoteDirSortFilterModel;
class RemoteDirSortFilterModelEX;
class LocalView;
class EncryptionDetailFocusLabel;
class Connection;
class RemoteView;

class FTPView : public RemoteView
{
    Q_OBJECT;
public:
    FTPView(QMdiArea *main_mdi_area, LocalView *local_view, QWidget *parent = 0);
    ~FTPView();

    QString get_selected_directory();
    //
    // void set_host_info(QString host_name, QString user_name, QString password, short port, QString pubkey);
    // void set_ssh2_handler(void *ssh2_sess, int ssh2_sock );
    // void set_user_home_path(std::string user_home_path);
    virtual void setConnection(Connection *conn);
    
    // bool is_in_remote_dir_retrive_loop() 
    // { return this->in_remote_dir_retrive_loop ; }
    
    void update_layout();
    
protected:
    void init_popup_context_menu();
    // LocalView  *local_view;
    // QMdiArea   *main_mdi_area;
    // QStatusBar *status_bar;
        
    // Ui::remoteview remoteview;
    // RemoteDirModel *remote_dir_model;
    // int   table_row_height ;
    // RemoteDirSortFilterModel *remote_dir_sort_filter_model;
    // RemoteDirSortFilterModelEX *remote_dir_sort_filter_model_ex;
    // QAbstractItemView *curr_item_view;    //
        
    // QMenu *dir_tree_context_menu ;
            
    // std::string user_home_path;
        
    // QCursor orginal_cursor;
    // bool    in_remote_dir_retrive_loop;
    // ProgressDialog *own_progress_dialog;
    
    // //
    // int     ssh2_sock;
    // LIBSSH2_SESSION *ssh2_sess;
    // LIBSSH2_CHANNEL *ssh2_channel;
    // LIBSSH2_SFTP *ssh2_sftp;
        
    // QString host_name;
    // QString user_name;
    // QString password;
    // short  port;
    // QString pubkey;
    // Connection *conn;
        
    // //menu item
    // QAction *attr_action;
    // EncryptionDetailFocusLabel *enc_label;
        
public slots:
    void i_init_dir_view();
    void slot_disconnect_from_remote_host();
        
    void slot_dir_tree_customContextMenuRequested(const QPoint &pos);
    void slot_new_transfer();
    void slot_new_upload_requested(TaskPackage local_pkg, TaskPackage remote_pkg);
    void slot_new_upload_requested(TaskPackage local_pkg);
    void slot_new_download_requested(TaskPackage local_pkg, TaskPackage remote_pkg);
    void slot_new_download_requested(TaskPackage remote_pkg);
        
    //////////ui
    void slot_show_fxp_command_log(bool show);
        
    void slot_custom_ui_area();
        
    void slot_enter_remote_dir_retrive_loop();
    void slot_leave_remote_dir_retrive_loop();
        
    void slot_transfer_finished(int status, QString errorString);
        
    //view drag
    void slot_drag_ready();
    
    //
    bool slot_drop_mime_data(const QMimeData *data, Qt::DropAction action,
			     int row, int column, const QModelIndex &parent ) ;
    void slot_show_hidden(bool show);
	
signals:
        
private slots:

    void slot_refresh_directory_tree();
    void slot_show_properties();
    void slot_mkdir();
    void slot_rmdir();        
    void slot_rename();
    void slot_copy_path();
    void slot_copy_url();
        
    void rm_file_or_directory_recursively();

    void slot_dir_tree_item_clicked( const QModelIndex & index);
    void slot_dir_file_view_double_clicked( const QModelIndex & index );

    void encryption_focus_label_double_clicked();
    void host_info_focus_label_double_clicked();
        
protected:
    virtual void closeEvent(QCloseEvent *event);
};

#endif