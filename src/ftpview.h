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

#include "remoteview.h"

class ProgressDialog;
class LocalView;
class EncryptionDetailFocusLabel;
class Connection;
class FTPConnection;
class RemoteView;

class FTPView : public RemoteView
{
    Q_OBJECT;
public:
    FTPView(QMdiArea *main_mdi_area, LocalView *local_view, QWidget *parent = 0);
    virtual ~FTPView();

    QString get_selected_directory();
    // 返回一个值不够用，一个地方需要同时检测两个值，trip_path, file_name,
    // 这个函数同时返回这两个值, first=strip_path, second=file_name
    QPair<QString, QString> get_selected_directory(bool pair);
    //
    // void set_user_home_path(std::string user_home_path);
    void setConnection(Connection *conn);
    
    // bool is_in_remote_dir_retrive_loop() 
    // { return this->in_remote_dir_retrive_loop ; }
    
    void update_layout();
    
protected:
    void init_popup_context_menu();

    FTPConnection *rconn; // real connection type;

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

    // log msg handler
    void slot_operation_triggered(QString text);

    void slot_dir_nav_go_home();
    void slot_dir_nav_prefix_changed(const QString &prefix);
    void slot_dir_nav_input_comfirmed(const QString &prefix);

    void onDirectoryLoaded(const QString &path);
	
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

    void onUpdateEntriesStatus();

    void encodingChanged();

protected:
    virtual void closeEvent(QCloseEvent *event);
    virtual QMenu *encodingMenu();
};

#endif
