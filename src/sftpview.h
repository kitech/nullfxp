/* sftpview.h --- 
 * 
 * Author: liuguangzhao
 * Copyright (C) 2007-2010 liuguangzhao@users.sf.net
 * URL: http://www.qtchina.net http://nullget.sourceforge.net
 * Created: 2008-05-31 22:09:15 +0800
 * Version: $Id: sftpview.h 567 2009-11-01 08:12:00Z liuguangzhao $
 */


#ifndef SFTPVIEW_H
#define SFTPVIEW_H

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
class SSHConnection;

class SFTPView : public RemoteView
{
    Q_OBJECT;
public:
    SFTPView(QMdiArea *main_mdi_area, LocalView *local_view, QWidget *parent = 0);
    virtual ~SFTPView();

    virtual QString get_selected_directory();
    //
    virtual void set_user_home_path(QString user_home_path);
    virtual void setConnection(Connection *conn);
    
    virtual bool is_in_remote_dir_retrive_loop() 
    { return this->in_remote_dir_retrive_loop ; }
    
    virtual void update_layout();
    
protected:
    SSHConnection *rconn;
    QStringListModel *m_operationLogModel;

    virtual void expand_to_home_directory(QModelIndex parent_model, int level);
    virtual void expand_to_directory(QString path, int level);

public slots:
    virtual void i_init_dir_view();
    virtual void slot_disconnect_from_remote_host();
        
    virtual void slot_dir_tree_customContextMenuRequested(const QPoint &pos);
    virtual void slot_new_transfer();
    virtual void slot_new_upload_requested(TaskPackage local_pkg, TaskPackage remote_pkg);
    virtual void slot_new_upload_requested(TaskPackage local_pkg);
    virtual void slot_new_download_requested(TaskPackage local_pkg, TaskPackage remote_pkg);
    virtual void slot_new_download_requested(TaskPackage remote_pkg);
        
    //////////ui
    virtual void slot_show_fxp_command_log(bool show);
        
    virtual void slot_custom_ui_area();
        
    virtual void slot_enter_remote_dir_retrive_loop();
    virtual void slot_leave_remote_dir_retrive_loop();
        
    virtual void slot_transfer_finished(int status, QString errorString);
        
    //view drag
    virtual void slot_drag_ready();
        
    //
    virtual bool slot_drop_mime_data(const QMimeData *data, Qt::DropAction action,
			     int row, int column, const QModelIndex &parent);
    virtual void slot_show_hidden(bool show);
	
    // log msg handler
    void slot_operation_triggered(QString text);

    virtual void slot_dir_nav_go_home();
    virtual void slot_dir_nav_prefix_changed(QString prefix);
    virtual void slot_dir_nav_input_comfirmed(QString prefix);

    virtual void onDirectoryLoaded(const QString &path);

signals:
        
private slots:
    virtual void slot_refresh_directory_tree();
    virtual void slot_show_properties();
    virtual void slot_mkdir();
    virtual void slot_rmdir();        
    virtual void slot_rename();
    virtual void slot_copy_path();
    virtual void slot_copy_url();
        
    virtual void rm_file_or_directory_recursively();

    virtual void slot_dir_tree_item_clicked( const QModelIndex &index);
    virtual void slot_dir_file_view_double_clicked( const QModelIndex & index );

    virtual void encryption_focus_label_double_clicked();
    virtual void host_info_focus_label_double_clicked();

    void onUpdateEntriesStatus();
        
protected:
    virtual void closeEvent(QCloseEvent *event);
};

#endif
