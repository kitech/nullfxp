/* localview.h --- 
 * 
 * Filename: localview.h
 * Description: 
 * Author: liuguangzhao
 * Maintainer: 
 * Copyright (C) 2007-2010 liuguangzhao <liuguangzhao@users.sf.net>
 * http://www.qtchina.net
 * http://nullget.sourceforge.net
 * Created: 六  5月 31 15:26:31 2008 (CST)
 * Version: 
 * Last-Updated: 二  7月 15 20:02:16 2008 (CST)
 *           By: 刘光照<liuguangzhao@users.sf.net>
 *     Update #: 1
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

#include "ui_localview.h"

class RemoteView ;


class LocalView : public QWidget
{
    Q_OBJECT
	public:
    LocalView(QWidget *parent = 0);

    ~LocalView();

    QString get_selected_directory();
    
    void update_layout();
    
signals:
    void new_upload_requested(QStringList local_file_names);
        
private:
    QStatusBar * status_bar ;
    QDirModel * model ;
    Ui::LocalView localView ;
    LocalDirSortFilterModel * dir_file_model ;
    int   table_row_height ;
    QAbstractItemView * curr_item_view ;    //
        
    void expand_to_home_directory(QModelIndex parent_model,int level );
        
    QMenu * local_dir_tree_context_menu ;
        
    void init_local_dir_tree_context_menu();
        
    public slots:
        
    void slot_local_dir_tree_context_menu_request(const QPoint & pos );
        
    void slot_local_new_upload_requested();
        
    void slot_refresh_directory_tree();
        
    void slot_show_hidden(bool show);
        
    private slots:
    void slot_dir_tree_item_clicked( const QModelIndex & index);
    void slot_dir_file_view_double_clicked( const QModelIndex & index );
    void slot_show_properties();
    void slot_mkdir();
    void slot_rmdir();
    void slot_remove();
    void slot_rename();
    void rm_file_or_directory_recursively();
    void slot_copy_path_url();
        
protected:
    virtual void closeEvent ( QCloseEvent * event );
};

#endif
