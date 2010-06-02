/* localview.h --- 
 * 
 * Author: liuguangzhao
 * Copyright (C) 2007-2010 liuguangzhao@users.sf.net
 * URL: http://www.qtchina.net http://nullget.sourceforge.net
 * Created: 2008-05-31 15:26:31 +0800
 * Version: $Id$
 */


#ifndef LOCALVIEW_H
#define LOCALVIEW_H

#include <QtCore>
#include <QtGui>
#include <QWidget>
#include <QMdiSubWindow>
#include <QTreeWidget>
#include <QDirModel>

// #include "localdirfilemodel.h"
#include "localdirsortfiltermodel.h"
#include "taskpackage.h"

#include "ui_localview.h"

class RemoteView ;

class LocalView : public QWidget
{
    Q_OBJECT;
public:
    LocalView(QWidget *parent = 0);
    ~LocalView();

    QString get_selected_directory();    
    void update_layout();
    
signals:
    void new_upload_requested(TaskPackage pkg);

public slots:        
    void slot_local_dir_tree_context_menu_request(const QPoint & pos);        
    void slot_local_new_upload_requested();        
    void slot_refresh_directory_tree();        
    void slot_show_hidden(bool show);

    void slot_dir_nav_go_home();
    void slot_dir_nav_prefix_changed(QString prefix);
        
private slots:
    void slot_dir_tree_item_clicked(const QModelIndex & index);
    void slot_dir_file_view_double_clicked(const QModelIndex & index);
    void slot_show_properties();
    void slot_mkdir();
    void slot_rmdir();
    void slot_remove();
    void slot_rename();
    void rm_file_or_directory_recursively();
    void slot_copy_path_url();

    void onUpdateEntriesStatus();
    void onDirectoryLoaded(const QString &path);
    void onFileRenamed(const QString &path,  const QString &oldName, const QString & newName);
    void onRootPathChanged(const QString &newPath);

protected:
    virtual void closeEvent ( QCloseEvent * event );
        
private:
    QStatusBar *status_bar;
    QFileSystemModel *model;
    Ui::LocalView uiw;
    LocalDirSortFilterModel *dir_file_model;
    int   table_row_height;
    QAbstractItemView *curr_item_view;    //
    QMenu *local_dir_tree_context_menu;

    bool is_dir_complete_request;
    QString dir_complete_request_prefix;
        
    void expand_to_home_directory(QModelIndex parent_model, int level);
    void init_local_dir_tree_context_menu();
};

#endif
