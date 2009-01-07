/* remoteview.h --- 
 * 
 * Filename: remoteview.h
 * Description: 
 * Author: liuguangzhao
 * Maintainer: 
 * Copyright (C) 2007-2008 liuguangzhao <liuguangzhao@users.sf.net>
 * http://www.qtchina.net
 * http://nullget.sourceforge.net
 * Created: 一  5月  5 22:09:15 2008 (CST)
 * Version: 
 * Last-Updated: 六  5月 31 15:25:48 2008 (CST)
 *           By: liuguangzhao
 *     Update #: 2
 * URL: 
 * Keywords: 
 * Compatibility: 
 * 
 */

/* Commentary: 
 * 
 * 
 * 
 */

/* Change log:
 * 
 * 
 */


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

class RemoteView : public QWidget
{
    Q_OBJECT;
public:
    RemoteView( QMdiArea * main_mdi_area , LocalView * local_view , QWidget *parent = 0);

    ~RemoteView();
    QString get_selected_directory();
    //
    void set_host_info ( QString host_name ,QString user_name, QString password, short port, QString pubkey);
    void set_ssh2_handler( void * ssh2_sess /*, void * ssh2_sftp*/, int ssh2_sock );
    void set_user_home_path(std::string user_home_path);
    
    bool is_in_remote_dir_retrive_loop() 
    { return this->in_remote_dir_retrive_loop ; }
    
    void update_layout();
    
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
    QString pubkey;
        
    //menu item
    QAction * attr_action;
    EncryptionDetailFocusLabel * enc_label;
        
public slots:
    void i_init_dir_view( );
    void slot_disconnect_from_remote_host();
        
    void slot_dir_tree_customContextMenuRequested ( const QPoint & pos );
    void slot_new_transfer();
    void slot_new_upload_requested(QStringList local_file_names, QStringList remote_file_names);
    void slot_new_upload_requested( QStringList local_file_names ) ;
    void slot_new_download_requested(QStringList local_file_names, QStringList remote_file_names);
    void slot_new_download_requested( QStringList remote_file_names ) ;
        
    //////////ui
    void slot_show_fxp_command_log(bool show);
        
    void slot_custom_ui_area();
        
    void slot_enter_remote_dir_retrive_loop();
    void slot_leave_remote_dir_retrive_loop();
        
    void slot_transfer_finished( int status, QString errorString ) ;
        
    //view drag
    void slot_drag_ready();
        
    //
    bool slot_drop_mime_data(const QMimeData *data, Qt::DropAction action,
			     int row, int column, const QModelIndex &parent ) ;
    void slot_show_hidden(bool show);
	
signals:
    void new_transfer_requested(QStringList local_file_names,QStringList remote_file_names);
        
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
    virtual void closeEvent ( QCloseEvent * event ) ;
};

#endif
